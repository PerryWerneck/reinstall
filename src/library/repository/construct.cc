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
 #include <reinstall/repository.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/xml.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	static inline Repository::Layout LayoutFactory(const pugi::xml_node &node) {
		return (Repository::Layout) XML::StringFactory(node,"layout","value","apache").select("apache","mirrorcache",nullptr);
	}

	Repository::Path::Path(const pugi::xml_node &node) : url{XML::QuarkFactory(node,"url").c_str()} {
	}

	Repository::Repository(const pugi::xml_node &node) : NamedObject(node), path(node), slp(node), layout{LayoutFactory(node)} {
	}

	Repository::~Repository() {
	}

	const std::string Repository::get_url(bool expand) {

		if(expand && slp) {
			std::string url = slp.get_url();
			if(!url.empty()) {
				return url;
			}
		}

		return path.url;

	}

 }

