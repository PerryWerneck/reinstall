/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
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
 #include <reinstall/source.h>
 #include <udjat/tools/protocol.h>
 #include <reinstall/dialogs.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <private/mirror.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	void Mirror::apache(const char *name, const char *path, const char *url, std::vector<std::shared_ptr<Source>> &contents) {

		// Get index and insert folder contents.
		string index = Protocol::WorkerFactory(url)->get();

		if(index.empty()) {
			throw runtime_error(Logger::Message(_("Empty response from {}"),url));
		}

		for(auto href = index.find("<a href=\""); href != string::npos; href = index.find("<a href=\"",href)) {

			auto from = href+9;
			href = index.find("\"",from);
			if(href == string::npos) {
				throw runtime_error(Logger::Message(_("Unable to parse file list from {}"),url));
			}

			string link = index.substr(from,href-from);

			if(link[0] =='/' || link[0] == '?' || link[0] == '.' || link[0] == '$')
				continue;

			if(link.size() >= 7 && strncmp(link.c_str(),"http://",7) == 0 ) {
				continue;
			}

			if(link.size() >= 8 && strncmp(link.c_str(),"https://",8) == 0 ) {
				continue;
			}

			string remote = url + link;
			string local = path + link;

			if(remote[remote.size()-1] == '/') {

				// Its a folder, expand it.
				Mirror::apache(name,local.c_str(),remote.c_str(),contents);

			} else {

				// Its a file, append in the content list.
				Logger::String {
					remote," -> ",local
				}.trace(name);
				contents.push_back(std::make_shared<Source>(name,remote.c_str(),local.c_str()));
			}

			href = from+1;

		}

	}

 }
