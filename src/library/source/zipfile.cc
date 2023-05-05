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

	ZipFile::ZipFile(const pugi::xml_node &node) : CachedFileSource{node} {

	}

	bool ZipFile::contents(const Action &action, std::vector<std::shared_ptr<Source>> &contents) {

		class ZipFileSource : public Reinstall::Source {
		public:
			ZipFileSource(const char *name, zip_t *zipfile, struct zip_stat &stat) : Reinstall::Source{name,"url",Quark{stat.name}.c_str()} {

				filenames.temp = File::Temporary::create();

#ifdef _WIN32
				int out = ::open(filenames.temp.c_str(),O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,0644);
#else
				int out = ::open(filenames.temp.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0644);
#endif // _WIN32

				zip_file *zf = zip_fopen_index(zipfile, stat.index, 0);

				Dialog::Progress &progress = Dialog::Progress::getInstance();
				progress.set_url(this->path);
				try {

					size_t sum = 0;
					char buffer[4096];
					while (sum != stat.size) {
						auto bufferlength = zip_fread(zf, buffer, 4096);

						if(::write(out,buffer,bufferlength) != bufferlength) {
							throw system_error(errno,system_category(),"Can't write image contents");
						}

						// fwrite(buffer, sizeof(char), bufferlength, file);
						sum += bufferlength;
						progress.set_progress((double) sum,(double) stat.size);
					}

				} catch(...) {

					::close(out);
					zip_fclose(zf);

					throw;
				}

                ::close(out);
				zip_fclose(zf);
				progress.set_url("");

			}

		};

		if(filenames.saved.empty()) {
			save();
		}

		zip_t *zipfile = zip_open(filenames.saved.c_str(),ZIP_RDONLY,NULL);
		if(!zipfile) {
			throw runtime_error("Cant open zipfile");
		}

		try {

			Dialog::Progress &progress = Dialog::Progress::getInstance();

			// https://gist.github.com/sdasgup3/a0255ebce3e3eec03e6878b47c8c7059
			auto entries = zip_get_num_entries(zipfile,0);
			for(zip_int64_t entry = 0; entry < entries; entry++) {

				struct zip_stat sb;

				progress.set_count(entry,entries);

				if (zip_stat_index(zipfile, entry, 0, &sb) != 0) {
					continue;
				}

				debug(entry," - ",sb.index," - ",sb.name);

				contents.push_back(make_shared<ZipFileSource>(name(),zipfile,sb));

			}

		} catch(...) {

			zip_close(zipfile);
			throw;

		}


		zip_close(zipfile);

		return true;
	}

#endif // HAVE_ZIPLIB

 }
