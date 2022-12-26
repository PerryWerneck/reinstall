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
 #include <udjat/defs.h>
 #include <udjat/version.h>
 #include <reinstall/source.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/configuration.h>
 #include <reinstall/dialogs.h>
 #include <udjat/tools/logger.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	Source::Source(const char *n, const char *u, const char *p)
		:	NamedObject(Quark(n).c_str()),
			url(Quark(u).c_str()),
			repository(Quark::getFromStatic("install").c_str()),
			path(Quark(p).c_str()) {
	}

	Source::Source(const pugi::xml_node &node,const Source::Type t,const char *url,const char *defpath)
		: 	NamedObject(node),
			type(t),
			url(Quark(node,"url",url,false).c_str()),
			repository(Quark(node,"repository","install").c_str()),
			path(Quark(node,"path",defpath,false).c_str()),
			message(Quark(node,"download-message","",true).c_str()) {

		if(!url[0]) {
			url = Quark(node,"path").c_str();
		}

		if(!url[0]) {
			throw runtime_error("Missing required attribute 'url'");
		}

	}

	Source::~Source() {
		if(!tempfilename.empty()) {
			if(remove(tempfilename.c_str()) != 0) {
				error() << "Error removing '" << tempfilename << "'" << endl;
			}
		}
	}

	void Source::set(const Action &action) {

		if(!url[0]) {
			// Expand URL based on repository path
			URL url(action.repository(repository)->url(true));
			url += path;
#if UDJAT_CORE_BUILD > 22122511
			url.expand();
#endif
			this->url = Quark(url.c_str()).c_str();
		}

		debug("URL=",this->url);

	}

 }

