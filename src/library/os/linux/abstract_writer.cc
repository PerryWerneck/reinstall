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

 #include <config.h>

 #include <reinstall/defs.h>
 #include <reinstall/writer.h>
 #include <system_error>
 #include <udjat/tools/intl.h>
 #include <unistd.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/configuration.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	void Writer::write(int fd, const void *buf, size_t length) {
		if(::write(fd,buf,length) != (ssize_t) length) {
			throw system_error(errno, system_category(),_("I/O error writing image"));
		}
	}

	void Writer::finalize(int fd) {
		::fsync(fd);
	}

	static const struct Worker {
		const char *name;
		const char *fsname;
		const std::function<void(const char *dev)> format;
	} workers[] = {
		{
			"fat32",
			"vfat",
			[](const char *dev) {

				Logger::String{"Formatting ",dev," on FAT32"}.trace("disk");

				// Format.
				SubProcess{
					Logger::Message{
						Config::Value<string>{
							"mkfs","fat32","/sbin/mkfs.vfat -F32 {}"
						}.c_str(),
						dev
					}.c_str()
				}.run();

			}
		}
	};

	static const Worker & WorkerFactory(const char *filesystemtype) {

		for(const Worker &worker : workers) {

			if(strcasecmp(filesystemtype,worker.name) == 0) {
				return worker;
			}

		}

		Logger::String{"Cant identify file system '",filesystemtype,"'"}.error(PACKAGE_NAME);

		throw runtime_error("Invalid file system type");
	}

	void Writer::format(const char *devname, const char *fsname) {
		WorkerFactory(fsname).format(devname);
	}

	void Writer::format(const char *) {
		Logger::String{"Pure virtual method '",__FUNCTION__,"' called on writer object"}.error(PACKAGE_NAME);
		throw system_error(ENOTSUP,system_category(),"Method not available");
	}

	/// @brief Get disk image.
	std::shared_ptr<Disk::Image> Writer::DiskImageFactory(const char *) {
		Logger::String{"Pure virtual method '",__FUNCTION__,"' called on writer object"}.error(PACKAGE_NAME);
		throw system_error(ENOTSUP,system_category(),"Method not available");
	}

 }
