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

 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>

 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <libreinstall/writer.h>
 #include <libreinstall/writers/usb.h>

 #include <system_error>
 #include <sys/inotify.h>
 #include <sys/ioctl.h>
 #include <sys/file.h>
 #include <linux/fs.h>
 #include <poll.h>
 #include <vector>
 #include <sys/sysmacros.h>

 #include <udjat/ui/dialog.h>
 #include <udjat/ui/dialogs/popup.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace std;
 using namespace Udjat;

 #define INOTIFY_EVENT_SIZE ( sizeof (struct inotify_event) )
 #define INOTIFY_EVENT_BUF_LEN ( 1024 * ( INOTIFY_EVENT_SIZE + 16 ) )

 namespace Reinstall {

	UsbWriter::UsbWriter(int f) : fd{f} {
	}

	UsbWriter::~UsbWriter() {
		if(fd > 0) {
			::close(fd);
		}
	}

	unsigned long long UsbWriter::size() {
		unsigned long long devlen = 0LL;
		if(ioctl(fd,BLKGETSIZE64,&devlen) < 0) {
			Logger::String{"Unable to get device length: ",strerror(errno)}.error("usb");
			return 0LL;
		}
		return devlen;
	}

	size_t UsbWriter::write(unsigned long long offset, const void *contents, size_t length) {
		throw runtime_error("Incomplete");
	}

	void UsbWriter::finalize() {
	}

	shared_ptr<Writer> UsbWriter::factory(const char *title, unsigned long long) {

		const Udjat::Dialog dialog {
			title,	// Title
			_("Insert an storage device <b>NOW</b> "), // Message
			_("This action will <b>DELETE ALL CONTENT</b> on the device."), // Secondary
		};

		const vector<Dialog::Button> buttons = {
			{ ECANCELED, 	_("Cancel"),	Dialog::Button::Standard 	},
			{ 0, 			_("Continue"),	Dialog::Button::Destructive },
		};

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

		// Detected device.
		struct {
			String name;
			int fd = -1;
		} device;

		int rc = -1;
		try {

			rc = dialog.run([fd,wd,&device](Dialog::Popup &popup){

				popup.disable(0);

				// Loop until user select device.
				while(popup) {

					struct pollfd pfd;
					pfd.fd = fd;
					pfd.events = POLLIN;
					pfd.revents = 0;

					int pfds = poll(&pfd, 1, 1000);

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

								if(event->wd == wd) {

									debug("Inotify event on '",event->name,"'");

									if(strncasecmp(event->name,"sd",2)) {

										Logger::String{"Ignoring device /dev/",event->name}.trace("usbwriter");

									} else if((event->mask & IN_CREATE) != 0) {

										debug("Device ",event->name," added");

										int devfd = ::open(String{"/dev/",event->name}.c_str(),O_RDWR);
										if(devfd < 0) {

											Logger::String{"Unable to open /dev/",event->name,": ",strerror(errno)}.trace("usbwriter");

										} else if(::flock(devfd, LOCK_EX|LOCK_NB) != 0) {

											Logger::String{"Error locking /dev/",event->name,": ",strerror(errno)}.trace("usbwriter");

										} else {

											// Got locked device, check type
											struct stat st;
											if(fstat(devfd,&st) == 0
												&& (st.st_mode & S_IFMT) == S_IFBLK
												&& major(st.st_rdev) == 8
												&& (minor(st.st_rdev) & 15) == 0
												) {

												device.name = event->name;
												device.fd = devfd;

												Logger::String{"Device /dev/",device.name," is valid"}.trace("usbwriter");
												popup.enable(0);

											} else {

												Logger::String{"Ignoring device /dev/",event->name}.trace("usbwriter");
												::close(devfd);
												devfd = -1;

											}
										}

									} else if((event->mask & IN_DELETE) != 0) {

										Logger::String{"Device ",event->name," removed"}.trace("usbwriter");
										if(device.fd > 0 && !strcmp(event->name,device.name.c_str())) {
											::close(device.fd);
											device.fd = -1;
											device.name.clear();
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
						throw system_error(errno,system_category(),"Error watching /dev");

					}

				}

				return 0;

			},buttons);

		} catch(...) {

			inotify_rm_watch(fd, wd);
			::close(fd);

			throw;
		}

		inotify_rm_watch(fd, wd);
		::close(fd);

		if(rc == ECANCELED) {
			throw runtime_error(_("Operation canceled by user"));
		}

		if(device.fd < 0) {
			throw runtime_error(_("Can detect USB storage device"));
		}

		Logger::String{"Creating writer for /dev/",device.name}.trace("usbwriter");
		return make_shared<UsbWriter>(device.fd);
	}

 }


	/*
 #include <config.h>
 #include <udjat/defs.h>
 #include <reinstall/writer.h>
 #include <string>
 #include <unistd.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <reinstall/userinterface.h>
 #include <reinstall/dialogs.h>
 #include <sys/inotify.h>
 #include <sys/poll.h>
 #include <sys/file.h>
 #include <list>
 #include <sys/stat.h>
 #include <sys/sysmacros.h>
 #include <linux/fs.h>



 namespace Reinstall {

	unsigned long long Writer::usbdevlength = 0;
	const char * Writer::usbdevname = nullptr;

	/// @brief Get the size of the device in fd.
	static unsigned long long devlen(int fd) {

		unsigned long long devlen = 0LL;
		if(ioctl(fd,BLKGETSIZE64,&device_len) < 0) {
			cerr << "usbstorage\tUnable to get storage length: " << strerror(errno) << endl;
			return 0;
		}

		return devlen;
	}

	std::shared_ptr<Writer> Writer::USBWriterFactory(const Reinstall::Action &action, size_t length) {

		if(!length) {
			length = usbdevlength;	// No length, use the command-line set.
		}

		if(usbdevname && *usbdevname) {

			if(length) {

				debug("Allocating ",length," bytes on ",usbdevname);

				int fd = ::open(usbdevname,O_CREAT|O_TRUNC|O_WRONLY,0644);
				if(fd < 0) {
					int err = errno;
					Logger::String{"Error opening '",usbdevname,"': ",strerror(err)," (rc=",err,")"}.error("usbdev");
					throw system_error(err,system_category(),"Cant create USB storage image");
				}

				if(fallocate(fd,0,0,length)) {
					int err = errno;
					::close(fd);
					Logger::String{"Error allocating '",usbdevname,"': ",strerror(err)," (rc=",err,")"}.error("usbdev");
					throw system_error(err,system_category(),"Cant allocate USB storage image");
				}

				::close(fd);
			}

			debug("Constructing usb file writer for '",usbdevname,"'");
			return make_shared<FileWriter>(action,usbdevname);
		}

		/// @brief USB storage writer.
		class Writer : public Reinstall::Writer {
		public:

			Writer(const Reinstall::Action &action) : Reinstall::Writer(action) {
			}

			/// @brief Device handler.
			struct Device {

				std::string name;
				int fd;
				bool locked = false;
				bool valid = false;					///< @brief Is this a valid block device?
				unsigned long long devlen = 0LL;	///< @brief The device length.

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

						struct stat st;
						if(fstat(fd,&st) == 0 && (st.st_mode & S_IFMT) == S_IFBLK) {

							// Is a block device, test if it's a disk or a partition.
							// https://www.kernel.org/doc/Documentation/admin-guide/devices.txt

							if(major(st.st_rdev) == 8) {

								if((minor(st.st_rdev) & 15) == 0) {

									valid = true;

									if(ioctl(fd,BLKGETSIZE64,&devlen) < 0) {
										cerr << "usbstorage\tUnable to get length of '" << name << "': " << strerror(errno) << endl;
									} else {
										cout << "usbstorage\tDevice '" << name << "' detected with id " << major(st.st_rdev) << " " << minor(st.st_rdev) << endl;
									}

								} else {

									valid = false;
									cout << "usbstorage\tPartition '" << name << "' detected with id " << major(st.st_rdev) << " " << minor(st.st_rdev) << endl;

								}

							} else {

								valid = false;
								cout << "usbstorage\tNon SCSI device '" << name << "' detected with id " << major(st.st_rdev) << " " << minor(st.st_rdev) << endl;

							}


						}

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

			bool detect() {

				for(Device & device : devices) {
					if(device.valid) {
						if(fd != device.fd) {
							if(fd == -1) {
								cout << "usbstorage\tSelecting device '" << device.name << "'" << endl;
							} else {
								cout << "usbstorage\tChanging device to '" << device.name << "'" << endl;
							}
						}
						fd = device.fd;
						return true;
					}
				}
				if(fd != -1) {
					cout << "usbstorage\tThe selected device is no longer available" << endl;
				}
				fd = -1;
				return false;
			}

			void format(const char *fsname) override {
				throw system_error(ENOTSUP,system_category(),"Method not available");
			}

			std::shared_ptr<Disk::Image> DiskImageFactory(const char *fsname) override {
				throw system_error(ENOTSUP,system_category(),"Method not available");
			}

			void open() override {
				debug("Starting image write");
				detect();
				if(!detect()) {
					throw runtime_error(_("Storage device is unavailable"));
				}
			}

			/// @brief Write data do device.
			void write(const void *buf, size_t count) override {
				super::write(fd,buf,count);
			}

			void finalize() override {
				debug("Finalizing");
				super::finalize(fd);
			}

			/// @brief Close Device.
			void close() override {
				debug("Closing image writer");
				devices.clear();
			}

		};

		std::shared_ptr<Writer> writer = std::make_shared<Writer>(action);

		int rc = -1;			//< @brief Callback return code (errno).
		int selected = -1;		//< @brief Selected device (for taskrunner).
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
			auto taskrunner = UserInterface::getInstance().TaskRunnerFactory(_("Insert an storage device <b>NOW</b> "),true);

			taskrunner->set_sub_title(_("This action will <b>DELETE ALL CONTENT</b> on the device."),true);

			int errcode = -1;

			auto cancel = taskrunner->ButtonFactory(_("Cancel"),[&errcode]{
				errcode = ECANCELED;
			});

			auto apply = taskrunner->ButtonFactory(_("Continue"),[&errcode]{
				errcode = 0;
			});
			apply->set_destructive();

			apply->disable();

			taskrunner->set(action);
			taskrunner->show();

			rc = taskrunner->push([taskrunner,&settings,fd,wd,&writer,&locked,&selected,&errcode,&apply](){

				// Watch for USB storage device to be detected.
				while(errcode == -1) {

					struct pollfd pfd;
					pfd.fd = fd;
					pfd.events = POLLIN;
					pfd.revents = 0;

					int pfds = poll(&pfd, 1,((locked || writer->devices.empty()) ? 500 : 100));

					debug(pfds," fd=",writer->fd," errcode=",errcode);
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

						writer->detect();

						if(writer->fd != selected) {
							selected = writer->fd;
							apply->enable(selected != -1);
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

				return errcode;

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
			writer.reset();
		}

		return writer;
	}

 }
	*/
