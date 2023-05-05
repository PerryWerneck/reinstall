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
 #include <pugixml.hpp>
 #include <udjat/tools/intl.h>
 #include <iostream>
 #include <udjat/tools/logger.h>

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

		if(filenames.saved.empty()) {
			save();
		}

		zip_t *zipfile = zip_open(filenames.saved.c_str(),ZIP_RDONLY,NULL);
		if(!zipfile) {
			throw runtime_error("Cant open zipfile");
		}

		try {

			// https://gist.github.com/sdasgup3/a0255ebce3e3eec03e6878b47c8c7059
			for(zip_int64_t entry = 0; entry < zip_get_num_entries(zipfile,0); entry++) {

				struct zip_stat sb;

				if (zip_stat_index(zipfile, entry, 0, &sb) != 0) {
					continue;
				}

				debug(sb.name);


			}

			throw runtime_error("Working");

		} catch(...) {

			zip_close(zipfile);
			throw;

		}


		zip_close(zipfile);

		return true;
	}

#endif // HAVE_ZIPLIB

 }
