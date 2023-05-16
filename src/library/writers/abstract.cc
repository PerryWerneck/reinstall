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
 #include <reinstall/defs.h>
 #include <reinstall/writer.h>
 #include <reinstall/diskimage.h>
 #include <system_error>
 #include <udjat/tools/intl.h>

#ifndef _WIN32
 #include <unistd.h>
#endif // _WIN32

 using namespace std;

 namespace Reinstall {

	Writer::Writer(const Reinstall::Action UDJAT_UNUSED(&action)) {
	}

	Writer::~Writer() {
	}

	void Writer::setUsbDeviceName(const char *name) {
		usbdevname = name;
	}

	void Writer::setUsbDeviceLength(unsigned long long length) {
		usbdevlength = length;
	}

	void Writer::open() {
	}

	void Writer::write(const void UDJAT_UNUSED(*buf), size_t UDJAT_UNUSED(count)) {
		throw system_error(ENOTSUP,system_category(),"Module is trying to write on dummy writer");
	}

	void Writer::finalize() {
	}

	void Writer::close() {
	}

	std::shared_ptr<Disk::Image> Writer::DiskImageFactory(const char *devname, const char *fsname) {
		return std::make_shared<Disk::Image>(devname,fsname);
	}

 }
