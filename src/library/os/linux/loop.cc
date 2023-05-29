/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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

 // References:
 //
 //	https://stackoverflow.com/questions/11295154/how-do-i-loop-mount-programmatically

 #include <config.h>
 #include <udjat/tools/intl.h>
 #include <unistd.h>
 #include "private.h"
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <linux/loop.h>
 #include <sys/ioctl.h>

 namespace Reinstall {

	Device::Loop::Loop() {

		//
		// Get a free loop device
		//
		fd.control = open("/dev/loop-control", O_RDWR);
		if(fd.control == -1) {
			throw system_error(errno, system_category(),"/dev/loop-control");
		}

		devnumber = ioctl(fd.control, LOOP_CTL_GET_FREE);

		if (devnumber == -1) {
			close(fd.control);
			throw system_error(errno, system_category(),_("Can't get an available loop device"));
		}

		std::string::assign("/dev/loop");
		std::string::append(to_string(devnumber));

	}

	Device::Loop::Loop(const char *filename) : Loop() {
		bind(filename);
	}

	void Device::Loop::bind(const char *filename) {

		unbind(); // Just in case.

		fd.image = open(filename, O_RDWR);
		if(fd.image < 0) {
			throw system_error(errno,system_category(),filename);
		}

		fd.device = open(c_str(), O_RDWR);
		if(fd.device == -1) {
			int err = errno;
			::close(fd.image);
			fd.image = -1;
			throw system_error(err, system_category(),c_str());
		}

		if (ioctl(fd.device, LOOP_SET_FD, fd.image) == -1) {
			throw system_error(errno, system_category(),c_str());
		}

		/*
		// Setup auto cleanup.

		struct loop_info loopinfo;
		memset(&loopinfo,0,sizeof(loopinfo));

		if (ioctl(fd.device, LOOP_GET_STATUS, &loopinfo) == -1) {
			throw system_error(errno, system_category(),c_str());
		}

		loopinfo.lo_flags |= LO_FLAGS_AUTOCLEAR;

		if (ioctl(fd.device, LOOP_SET_STATUS, &loopinfo) == -1) {
			throw system_error(errno, system_category(),c_str());
		}
		*/

	}

	void Device::Loop::unbind() {

		if(fd.image > 0) {
			::close(fd.image);
			fd.image = -1;
		}

		if(fd.device > 0) {
			ioctl(fd.device, LOOP_CLR_FD, 0);
			::close(fd.device);
			fd.device = -1;
		}

	}

	Device::Loop::~Loop() {

		unbind();

		for(size_t i = 0; i < 200; i++) {

			if(ioctl(fd.control, LOOP_CTL_REMOVE, devnumber) >= 0) {
				break;
			}

			if(errno == EBUSY) {
				Logger::String{"Device ",c_str()," is busy (retry ",i,"/200)"}.trace("loop");
			} else {
				Logger::String{"Error '",strerror(errno),"' releasing device (retry ",i,"/200)",c_str()}.error("loop");
			}
			sleep(10);

		}

		::close(fd.control);
	}

 }

