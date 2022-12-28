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

	std::shared_ptr<Writer> Writer::FileFactory(const char *filename) {

		class Writer : public Reinstall::Writer {
		private:
			int fd = -1;
			string filename;

		public:
			Writer(const char *fn) : filename{fn} {

				if(filename.empty()) {
					throw runtime_error("Invalid filename");
				}

				debug("Writer for '",filename.c_str(),"' was constructed");
			}

			virtual ~Writer() {
				if(fd > 0) {
					close();
				}
			}

			void open() override {
				debug("Opening '",filename.c_str(),"'");
				fd = ::open(filename.c_str(),O_CREAT|O_TRUNC|O_APPEND|O_WRONLY,0666);
				if(fd < 0) {
					throw system_error(errno,system_category(),filename);
				}
			}

			void close() override {
				if(fd > 0) {
					debug("Closing '",filename.c_str(),"'");
					::close(fd);
				}
				fd = -1;
			}

			void finalize() override {
				::fsync(fd);
			}

			void write(const void *buf, size_t length) {

				if(::write(fd,buf,length) != (ssize_t) length) {
					throw system_error(errno, system_category(),_("I/O error writing image"));
				}

			}

		};

		return make_shared<Writer>(filename);
	}


 }
