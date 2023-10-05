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
 #include <udjat/tools/configuration.h>
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

	UsbWriter::UsbWriter(int fd) : Udjat::File::Handler{fd} {
	}

	UsbWriter::~UsbWriter() {
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
		return Udjat::File::Handler::write(offset,contents,length);
	}

	void UsbWriter::finalize() {
		::fsync(fd);
	}

	shared_ptr<Writer> UsbWriter::factory(const char *title, unsigned long long) {
	
		const Udjat::Dialog dialog {
			title,	// Title
			_("Insert an storage device <b>NOW</b> "), // Message
			_("This action will <b>DELETE ALL CONTENT</b> on the device."), // Secondary
		};

		Config::Value<string> cancel{"usb","cancel",_("Cancel")};

		const vector<Dialog::Button> buttons = {
			{
				ECANCELED,
				cancel.c_str(),
				Dialog::Button::Standard
			},
			{
				0,
				_("Continue"),
				Dialog::Button::Destructive
			},
		};

		// TODO: Use libudev

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

				Config::Value<string> detecting{"usb","detecting",_("Detecting device")};
				Config::Value<string> confirm{"usb","continue",_("Continue")};

				popup.disable(0);
				popup.set_label(0,detecting.c_str());

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

												Logger::String{"Device /dev/",device.name," is valid"}.trace("usbwriter");
												if(device.fd > 0) {
													::close(device.fd);
													Logger::String{"Replacing ",device.name," with ",event->name}.warning("usbwriter");
												}

												device.name = event->name;
												device.fd = devfd;

												popup.set_label(0,confirm.c_str());
												popup.enable(0);

											} else {

												// Lock failed.
												if(devfd > 0) {
													::close(devfd);
												}

												Logger::String{"Ignoring device /dev/",event->name}.trace("usbwriter");

											}
										}

									} else if((event->mask & IN_DELETE) != 0) {

										Logger::String{"Device ",event->name," removed"}.trace("usbwriter");
										if(device.fd > 0 && !strcmp(event->name,device.name.c_str())) {
											::close(device.fd);
											device.fd = -1;
											popup.set_label(0,detecting.c_str());
											popup.disable(0);
											device.name.clear();
										}

									}

								}

								// Get next entry
								bufPtr += (offsetof (struct inotify_event, name) + event->len);

							}

							bytes = ::read(fd, buffer, INOTIFY_EVENT_BUF_LEN);

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


