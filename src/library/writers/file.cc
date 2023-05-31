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
 #include <reinstall/action.h>
 #include <string>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <system_error>
 #include <unistd.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	FileWriter::FileWriter(const Reinstall::Action &action, const char *fn) : Reinstall::Writer(action), filename{fn} {

		if(filename.empty()) {
			throw runtime_error("Invalid filename");
		}

		Logger::String{"Writer for '",filename.c_str(),"' was constructed"}.trace(PACKAGE_NAME);
	}

	FileWriter::~FileWriter() {
		if(fd > 0) {
			close();
		}
		Logger::String{"Writer for '",filename.c_str(),"' was destroyed"}.trace(PACKAGE_NAME);
	}

	void FileWriter::open() {
		Logger::String{"Writer for '",filename.c_str(),"' was open"}.trace(PACKAGE_NAME);
		fd = ::open(filename.c_str(),O_CREAT|O_TRUNC|O_APPEND|O_RDWR,0666);
		if(fd < 0) {
			throw system_error(errno,system_category(),filename);
		}
	}

	void FileWriter::format(const char *fsname) {
		super::format(filename.c_str(),fsname);
	}

	void FileWriter::make_partition(uint64_t length, const char *parttype) {
		Reinstall::Writer::make_partition(fd,length,parttype);
	}

	std::shared_ptr<Disk::Image> FileWriter::DiskImageFactory(const char *fsname) {
		return super::DiskImageFactory(filename.c_str(),fsname);
	}

	void FileWriter::close() {
		if(fd > 0) {
			Logger::String{"Writer for '",filename.c_str(),"' was closed"}.trace(PACKAGE_NAME);
			::close(fd);
		}
		fd = -1;
	}

	void FileWriter::finalize() {
		Reinstall::Writer::finalize(fd);
	}

	void FileWriter::write(const void *buf, size_t length) {
		Reinstall::Writer::write(fd,buf,length);
	}

	std::shared_ptr<Writer> Writer::FileWriterFactory(const Reinstall::Action &action, const char *filename) {

		return make_shared<FileWriter>(action,filename);
	}


 }
