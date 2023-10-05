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
 #include <libreinstall/writer.h>
 #include <libreinstall/writers/file.h>
 #include <udjat/tools/file/handler.h>
 #include <udjat/tools/logger.h>
 #include <stdexcept>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

 	FileWriter::FileWriter(int fd) : Udjat::File::Handler{fd} {
 	}

 	FileWriter::FileWriter(const char *filename) : Udjat::File::Handler{filename,true} {
 		Logger::String{"Writer to '",filename,"' was initialized"}.trace(PACKAGE_NAME);
 	}

 	FileWriter::~FileWriter() {
 	}

	size_t FileWriter::write(unsigned long long offset, const void *contents, size_t length) {
		return Udjat::File::Handler::write(offset,contents,length);
	}

	void FileWriter::finalize() {
		if(fsync(fd)) {
			Logger::String{"Unexpected error on fsync: ",strerror(errno)}.error("Writer");
			throw runtime_error("Unexpected error finalizing file write");
		}

 	}

 }
