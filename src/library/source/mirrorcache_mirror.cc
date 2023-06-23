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

 /*
 #include <reinstall/source.h>
 #include <udjat/tools/protocol.h>
 #include <reinstall/dialogs.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <private/mirror.h>
 #include <json/json.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	void Mirror::mirrorcache(const char *name, const char *path, const char *url, std::vector<std::shared_ptr<Source>> &contents) {

		Json::Value value;

		// Load JSON value.
		{
			string table = Protocol::WorkerFactory( (string{url} + "?jsontable").c_str() )->get();

			Json::CharReaderBuilder builder;
			JSONCPP_STRING err;

			const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

			const char *text = table.c_str();
			if (!reader->parse(text, text+strlen(text), &value, &err)) {
				throw runtime_error(err);
			}
		}

		for(Json::Value &item : value["data"]) {

			std::string link{item["name"].asCString()};

			if(link.empty()) {
				continue;
			}

			string remote = url + link;
			string local = path + link;

			if(remote[remote.size()-1] == '/') {

				// Its a folder, expand it.
				mirrorcache(name,local.c_str(),remote.c_str(),contents);

			} else {

				// Its a file, append in the content list.
				Logger::String {
					remote," -> ",local
				}.trace(name);

				contents.push_back(std::make_shared<Source>(name,remote.c_str(),local.c_str()));
			}

		}

	}

 }
 */
