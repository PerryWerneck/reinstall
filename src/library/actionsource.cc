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

 #include "private.h"
 #include <reinstall/action.h>
 #include <reinstall/worker.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/url.h>

 namespace Reinstall {

	Action::Source::Source(const pugi::xml_node &node,const char *url,const char *defpath)
		: 	url(Quark(node,"url",url,false).c_str()),
			path(Quark(node,"path",defpath,false).c_str()),
			message(Quark(node,"download-message","",true).c_str()) {

		if(!*url && path[0] == '/') {
			Udjat::URL install(getAttribute(node,"defaults","install",""));
			if(!install.empty()) {
				install += path;
				url = Quark(install).c_str();
			}
		}

		if(!*url) {
			throw runtime_error("Missing required attribute 'url'");
		}

	}

	Action::Source::~Source() {
		if(!tempfilename.empty()) {
			if(remove(tempfilename.c_str()) != 0) {
				cerr << "tempfile\tError removing '" << tempfilename << "'" << endl;
			}
		}
	}

	string Action::Source::save() {

		auto worker = Udjat::Protocol::WorkerFactory(this->url);

		if(filename) {

			// Download URL to 'filename'
			worker->save(filename);
			return filename;

		} else if(tempfilename.empty()) {

			// Download to temporary file.
			tempfilename = worker->save();
			filename = tempfilename.c_str();

		}

		return string(filename);

	}

 }

