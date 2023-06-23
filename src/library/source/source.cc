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

#ifndef _WIN32
	#include <limits.h>
	#include <cstdlib>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h>
#endif // _WIN32

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	Source::Source(const char *n, const char *u, const char *p)
		:	NamedObject{Quark(n).c_str()},
			url{Quark(u).c_str()},
			repository{Quark::getFromStatic("install").c_str()},
			path{Quark(p).c_str()} {

		if(!url[0]) {
			throw runtime_error(string{"Missing required attribute 'url' on node "} + name());
		}

		if(url[0] == '/' && !path[0]) {
			path = url;
		}

	}

	Source::Source(const pugi::xml_node &node,const Source::Type t,const char *defurl,const char *defpath)
		: 	NamedObject{node},
			type{t},
			url{getAttribute(node,"url",defurl)},
			repository{getAttribute(node,"repository","install")},
			path{getAttribute(node,"path",defpath)},
			message{getAttribute(node,"download-message","")} {

		if(!url[0]) {
			throw runtime_error(string{"Missing required attribute 'url' on node "} + name());
		}

		if(url[0] == '/' && !path[0]) {
			path = url;
		}

	}

	Source::~Source() {
		if(!filenames.temp.empty()) {
			if(remove(filenames.temp.c_str()) != 0) {
				error() << "Unable to remove '" << filenames.temp << "': " << strerror(errno) << endl;
			}
		}
	}

	void Source::set(const Reinstall::Action &object) {

		Udjat::String expander;

		// Expand URL.
		{
			expander = this->url;
			expander.expand(object);

			debug(expander);

			if(expander[0] == '/' || expander[0] == '.') {

				// Expand URL based on repository path
				URL url{object.repository(repository)->get_url(true)};
				url += expander.c_str();
				expander = url.c_str();
				expander.expand(object);

			} else if(strncasecmp(expander.c_str(),"relurl://",9) == 0) {

				URL url{object.repository(repository)->get_url(true)};
				url += (expander.c_str()+9);
				expander = url.c_str();
				expander.expand(object);

			}

			if(strcmp(expander.c_str(),this->url)) {
				this->url = Quark{expander}.c_str();
			}

			debug("URL=",this->url);
		}

		// Expand path
		if(this->path && this->path[0]) {
			expander = this->path;
			expander.expand(object);
			if(strcmp(expander.c_str(),this->path)) {
				this->path = Quark{expander}.c_str();
			}
		}

	}

#ifndef _WIN32
	const std::string Source::fspath(const char *path) {

		char buffer[PATH_MAX+1];
		memset(buffer,0,PATH_MAX+1);

		if(!realpath(path,buffer)) {
			throw system_error(errno, system_category(),path);
		}

		// Get source device.
		struct stat st;
		dev_t st_dev;

		if(stat(buffer,&st)) {
			throw system_error(errno, system_category(),buffer);
		}
		st_dev = st.st_dev;
		char *ptr = buffer;

		debug("'",buffer,"' is on device ",((int) st.st_dev));

		while(st_dev == st.st_dev) {

			if(!*buffer) {
				debug("Mount point for '",path,"' is '/'");
				return realpath(path,buffer);
			}

			ptr = strrchr(buffer,'/');
			if(!ptr) {
				throw runtime_error("Error parsing path delimiter");
			}
			*ptr = 0;

			if(stat((*buffer ? buffer : "/"),&st)) {
				throw system_error(errno, system_category(),buffer);
			}

		}

		*ptr = '/';

		debug("Mount point for '",path,"' is '",buffer,"'");
		return path + strlen(buffer);

	}
#endif // !_WIN32

 }
*/
