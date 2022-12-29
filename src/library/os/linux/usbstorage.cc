/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2022 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #include <config.h>
 #include <udjat/defs.h>
 #include <reinstall/writer.h>
 #include <string>
 #include <system_error>
 #include <unistd.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <reinstall/userinterface.h>
 #include <reinstall/dialogs.h>
 #include <sys/inotify.h>
 #include <sys/poll.h>
 #include <sys/file.h>
 #include <list>

 using namespace std;
 using namespace Udjat;

 #define INOTIFY_EVENT_SIZE ( sizeof (struct inotify_event) )
 #define INOTIFY_EVENT_BUF_LEN ( 1024 * ( INOTIFY_EVENT_SIZE + 16 ) )

 namespace Reinstall {

	std::shared_ptr<Writer> Writer::USBStorageFactory(size_t length) {

		/// @brief USB storage writer.
		class Writer : public Reinstall::Writer {
		public:

			/// @brief Device handler.
			struct Device {

				std::string name;
				int fd;
				bool locked = false;

				Device(Device &src) = delete;

				Device(const char *n) : name{n}, fd{::open((string{"/dev/"}+n).c_str(),O_RDWR)} {

					if(fd == -1) {
						cerr << "usbstorage\tError '" << strerror(errno) << "' opening " << name << endl;
					} else {
						lock();
					}

				}

				bool lock() {
					//
					// These two concepts allow tools such as disk partitioners or
					// file system formatting tools to safely and easily take exclusive
					// ownership of a block device while operating: before starting work
					// on the block device, they should take an LOCK_EX lock on it.
					//
					// https://systemd.io/BLOCK_DEVICE_LOCKING/
					//
					if(fd == -1 || locked) {
						return true;
					}
					if(::flock(fd, LOCK_EX|LOCK_NB) == 0) {
						cout << "usbstorage\tGot lock on '" << name << "'" << endl;
						locked = true;
					};
					return locked;
				}

				~Device() {
					if(fd != -1) {
						::close(fd);
						fd = -1;
						if(locked) {
							cout << "usbstorage\tDevice '" << name << "' was unlocked" << endl;
						}
					}
				}

			};

			std::list<Device> devices;

			int fd = -1;	///< @brief Handle to selected device.

			void open() override {
			}

			/// @brief Write data do device.
			void write(const void *buf, size_t count) override {
			}

			void finalize() override {
			}

			/// @brief Close Device.
			void close() override {
				devices.clear();
			}

		};

		std::shared_ptr<Writer> writer = std::make_shared<Writer>();

		int rc = -1;			//< @brief Callback return code (errno).
		bool locked = false;	//< @brief All devices are locked?

		//
		// Watch /dev
		//
		int fd = inotify_init1(IN_NONBLOCK|IN_CLOEXEC);
		if(fd == -1) {
			throw system_error(errno,system_category(),"Can't initialize inotify");
		}

		int wd = inotify_add_watch(fd,"/dev",IN_CREATE|IN_DELETE);
		if(fd == -1) {
			int err = errno;
			::close(fd);
			 throw system_error(err,system_category(),"Unable to watch /dev");
		}

		try {

			//
			// Settings.
			//
			struct {
				std::string device;
			} settings;

			//
			// First wait for an storage device
			//
			auto taskrunner = UserInterface::getInstance().TaskRunnerFactory();

			taskrunner->set_title(_("Insert <b>NOW</b> an storage device"));
			taskrunner->set_sub_title(_("The contents of inserted device will be <b>ALL ERASED</b>! "));
			taskrunner->allow_continue(false);

			rc = taskrunner->push([taskrunner,&settings,fd,wd,&writer,&locked](){

				// Watch for USB storage device to be detected.
				while(taskrunner->enabled()) {

					struct pollfd pfd;
					pfd.fd = fd;
					pfd.events = POLLIN;
					pfd.revents = 0;

					int pfds = poll(&pfd, 1,((locked || writer->devices.empty()) ? 500 : 100));

					debug(pfds);
					if(pfds == 0) {

						// Timeout, check for locks.
						if(!locked) {
							locked = true;
							for(Writer::Device & device : writer->devices) {
								if(!device.lock()) {
									locked = false;
								}
							}
						}

					} else if(pfds == 1 && (pfd.revents & POLLIN)) {

						// Got response.
						char buffer[INOTIFY_EVENT_BUF_LEN];
						memset(buffer,0,INOTIFY_EVENT_BUF_LEN);

						ssize_t bytes = ::read(fd, buffer, INOTIFY_EVENT_BUF_LEN);

						while(bytes > 0) {

							ssize_t bufPtr  = 0;

							while(bufPtr < bytes) {

									// Get event
									auto event = (struct inotify_event *) &buffer[bufPtr];

									if(event->wd == wd) {

										debug("Inotify event on '",event->name,"'");

										if(strncmp(event->name,"sd",2) == 0) {

											if((event->mask & IN_CREATE) != 0) {

												// Device was created.
												writer->devices.emplace_back(event->name);
												locked = false;

											} else if((event->mask & IN_DELETE) != 0) {

												cout << "usbstorage\tDevice " << event->name << " was removed" << endl;

												writer->devices.remove_if([event](const Writer::Device &device){
													return strcmp(device.name.c_str(),event->name) == 0;
												});

											}

										}
									}

									// Get next entry
									bufPtr += (offsetof (struct inotify_event, name) + event->len);
							}

							bytes = read(fd, buffer, INOTIFY_EVENT_BUF_LEN);

						}

					} else if(pfds < 0) {

						// Error.
						cerr << "usbstorage\tError '" << strerror(errno) << "' waiting for inotify socket" << endl;
						return errno;
					}

				}

				return ECANCELED;

			});

		} catch(...) {

			inotify_rm_watch(fd, wd);
			::close(fd);

			throw;
		}

		inotify_rm_watch(fd, wd);
		::close(fd);

		if(rc) {
			clog << "usbstorage\tWatcher finished with error '" << strerror(rc) << "' (rc=" << rc << ")" << endl;
			return std::shared_ptr<Writer>();
		}


		//
		// Got device
		//


		return writer;
	}


 }
