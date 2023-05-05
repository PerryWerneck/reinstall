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
 #include <reinstall/repository.h>
 #include <udjat/tools/protocol.h>
 #include <reinstall/dialogs.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <private/mirror.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	bool Source::contents(const Action &action, std::vector<std::shared_ptr<Source>> &contents) {

		URL url{this->url};

		if( *(url.c_str() + url.size() - 1) != '/') {
			return false;
		}

		if(url[0] == '/') {

			if(!(repository && *repository)) {
				throw runtime_error(string{"A repository attribute is required to expand url '"} + url + "'");
			}

			url = action.repository(repository)->url(true);
			url += this->url;

			debug("url=",this->url," expanded=",url.c_str());
		}

		if(message && *message) {
			Dialog::Progress::getInstance().set_title(message);
		}

		Repository::Layout layout = Repository::ApacheLayout;
		if(repository && *repository) {
			layout = action.repository(repository)->layout;
		}

		switch(layout) {
		case Repository::ApacheLayout:
			debug("Loading contents from '",url.c_str(),"' in apache format");
			Mirror::apache(name(),path,url.c_str(),contents);
			break;

		case Repository::MirrorCacheLayout:
			debug("Loading contents from '",url.c_str(),"' in MirrorCache format");
			Mirror::mirrorcache(name(),path,url.c_str(),contents);
			break;

		default:
			throw runtime_error("The repository layout is invalid");
		}

		debug("Source ",name()," was loaded");

		return true;
	}

 }
