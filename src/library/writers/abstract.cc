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
 #include <system_error>
 #include <udjat/tools/intl.h>

#ifndef _WIN32
 #include <unistd.h>
#endif // _WIN32

 using namespace std;

 namespace Reinstall {

	Writer::Writer() {
	}

	Writer::~Writer() {
	}

#ifndef _WIN32

	void Writer::write(int fd, const void *buf, size_t length) {
		if(::write(fd,buf,length) != (ssize_t) length) {
			throw system_error(errno, system_category(),_("I/O error writing image"));
		}
	}

	void Writer::finalize(int fd) {
		::fsync(fd);
	}

#endif // _WIN32

 }
