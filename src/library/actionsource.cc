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
 #include <udjat/tools/object.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/configuration.h>
 #include <reinstall/dialogs.h>

 namespace Reinstall {

	Action::Source::Source(const char *u, const char *p)
		:	url(Quark(u).c_str()),
			path(Quark(p).c_str()) {
	}

	Action::Source::Source(const pugi::xml_node &node,const char *url,const char *defpath)
		: 	url(Quark(node,"url",url,false).c_str()),
			path(Quark(node,"path",defpath,false).c_str()),
			message(Quark(node,"download-message","",true).c_str()) {


		if(!this->url[0] && path[0] == '/') {

			Udjat::URL install(Udjat::String(Udjat::Attribute(node,"install").as_string()).expand(node));

			if(install.empty()) {
				install = Udjat::String(Udjat::Config::get("defaults","install","")).expand(node);
			}

			if(!install.empty()) {
				install += path;
				this->url = Quark(install).c_str();
			}

		}

		if(!this->url[0]) {
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

		Dialog::Progress &progress = Dialog::Progress::getInstance();
		auto worker = Udjat::Protocol::WorkerFactory(this->url);

		if(filename) {

			// Download URL to 'filename'
			progress.set(worker->url().c_str());
			worker->save(filename,[&progress](double current, double total){
				progress.update(current,total);
				return true;
			});

			return filename;

		} else if(tempfilename.empty()) {

			// Download to temporary file.
			tempfilename = worker->save([&progress](double current, double total){
				progress.update(current,total);
				return true;
			});

			filename = tempfilename.c_str();

		}

		return string(filename);

	}

 }

