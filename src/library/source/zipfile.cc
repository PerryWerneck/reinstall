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
 #include <reinstall/source.h>
 #include <reinstall/sources/zipfile.h>
 #include <reinstall/dialogs/progress.h>
 #include <pugixml.hpp>
 #include <udjat/tools/intl.h>
 #include <iostream>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/file.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>

#ifdef HAVE_ZIPLIB
	#include <zip.h>
#endif // HAVE_ZIPLIB

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

#ifdef HAVE_ZIPLIB

	ZipFile::ZipFile(const pugi::xml_node &node) : Source{node} {

	}

	bool ZipFile::contents(const Action &action, std::vector<std::shared_ptr<Source>> &contents) {

		struct Container {

			std::string filename;
			zip_t *handler;

			Container(const char *name) : filename{name}, handler{zip_open(name,ZIP_RDONLY,NULL)} {
				Logger::String{"Opening ",filename}.trace("zip");
				if(!handler) {
					throw runtime_error(string("Cant open '") + filename + "'");
				}
			}

			~Container() {
				Logger::String{"Closing ",filename}.trace("zip");
				zip_close(handler);
			}

		};

		class ZipFileSource : public Reinstall::Source {
		private:
			shared_ptr<Container> container;
			struct zip_stat file;

		public:
			ZipFileSource(shared_ptr<Container> c, const char *name, struct zip_stat &f)
				: Reinstall::Source{name,Quark{string{"zip:///"}+f.name}.c_str(),Quark{f.name}.c_str()}, container{c}, file{f} {
			}

			void save(const std::function<void(const void *buf, size_t length)> &write) override {

				zip_file *zf = zip_fopen_index(container->handler, file.index, 0);

				Dialog::Progress &progress = Dialog::Progress::getInstance();
				progress.set_url(this->path);

				try {

					size_t sum = 0;
					char buffer[4096];
					while (sum != file.size) {
						auto bufferlength = zip_fread(zf, buffer, 4096);
						write(buffer,bufferlength);
						sum += bufferlength;
						progress.set_progress((double) sum,(double) file.size);
					}

				} catch(...) {
					zip_fclose(zf);
					throw;
				}

				zip_fclose(zf);
				progress.set_url("");

			}

			void save(const char *filename) override {

				debug(path," -> ",filename);

#ifdef _WIN32
				int out = ::open(filename,O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,0644);
#else
				int out = ::open(filename,O_WRONLY|O_CREAT|O_TRUNC,0644);
#endif // _WIN32

				try {

					save([out](const void *buffer, size_t length){
						if(::write(out,buffer,length) != length) {
							throw system_error(errno,system_category(),"Can't write image contents");
						}
					});

				} catch(...) {

					Logger::String{"Download of ",filename," was aborted"};
					::close(out);
					throw;
				}

				::close(out);


				container.reset();

			}

		};

		if(filenames.saved.empty()) {
			save();
		}

		shared_ptr<Container> container = make_shared<Container>(filenames.saved.c_str());

		// https://gist.github.com/sdasgup3/a0255ebce3e3eec03e6878b47c8c7059
		auto entries = zip_get_num_entries(container->handler,0);
		for(zip_int64_t entry = 0; entry < entries; entry++) {

			struct zip_stat sb;

			if (zip_stat_index(container->handler, entry, 0, &sb) != 0 || *(sb.name+strlen(sb.name)-1) == '/')  {
				continue;
			}

			debug(entry," - ",sb.index," - ",sb.name);

			contents.push_back(make_shared<ZipFileSource>(container,name(),sb));

		}

		return true;
	}

#endif // HAVE_ZIPLIB

 }
