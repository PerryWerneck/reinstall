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

 using namespace std;
 using namespace Udjat;

 #define INOTIFY_EVENT_SIZE ( sizeof (struct inotify_event) )
 #define INOTIFY_EVENT_BUF_LEN ( 1024 * ( INOTIFY_EVENT_SIZE + 16 ) )

 namespace Reinstall {

	std::shared_ptr<Writer> Writer::USBStorageFactory() {

		int dd = -1;	//< @brief Device descriptor.

		//
		// Watch /dev
		//
		int fd = inotify_init1(IN_NONBLOCK|IN_CLOEXEC);
		if(fd == -1) {
			throw system_error(errno,system_category(),"Can't initialize inotify");
		}

		int wd = inotify_add_watch(fd,"/dev",IN_CREATE);
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

			taskrunner->push([taskrunner,&settings,fd,wd,&dd](){

				// Watch for USB storage device to be detected.
				while(taskrunner->enabled()) {

					struct pollfd pfd;
					pfd.fd = fd;
					pfd.events = POLLIN;
					pfd.revents = 0;

					int pfds = poll(&pfd, 1, 300);

					if(pfds == 1 && (pfd.revents & POLLIN)) {

						// Got response.
						char buffer[INOTIFY_EVENT_BUF_LEN];
						memset(buffer,0,INOTIFY_EVENT_BUF_LEN);

						ssize_t bytes = ::read(fd, buffer, INOTIFY_EVENT_BUF_LEN);

						while(bytes > 0) {

							ssize_t bufPtr  = 0;

							while(bufPtr < bytes) {

									// Get event
									auto event = (struct inotify_event *) &buffer[bufPtr];

									if(event->wd == wd && (event->mask & IN_CREATE) != 0) {

										// Process event
										string name{event->name,event->len-1};
										debug("Inotify event on '",name.c_str(),"'");

									}

									// Get next entry
									bufPtr += (offsetof (struct inotify_event, name) + event->len);
							}

							bytes = read(fd, buffer, INOTIFY_EVENT_BUF_LEN);

						}


					} else if(pfds < 0) {

						// Error.
						cerr << "usbstorage\tError '" << strerror(errno) << "' waiting for inotify socket" << endl;
						break;
					}

				}

			});

		} catch(...) {

			inotify_rm_watch(fd, wd);
			::close(fd);

			throw;
		}

		inotify_rm_watch(fd, wd);
		::close(fd);

		//
		// Got device
		//

		/*
		class Writer : public Reinstall::Writer {
		private:

		public:
			Writer() {


			}

			virtual ~Writer() {
			}

			void open() override {
			}

			void close() override {
			}

			void finalize() override {
			}

			void write(const void *buf, size_t length) {
			}

		};

		return make_shared<Writer>();
		*/

	}


 }
