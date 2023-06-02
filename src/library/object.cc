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
 #include <udjat/tools/object.h>
 #include <reinstall/object.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/logger.h>
 #include <pugixml.hpp>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/application.h>
 #include <iostream>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	pugi::xml_node Abstract::Object::find(const pugi::xml_node &node, const char *attrname) {

		for(pugi::xml_node child = node.child("attribute"); child; child = child.next_sibling("attribute")) {
			const char *name = child.attribute("name").as_string("");
			if(!strcasecmp(name,attrname)) {
				return child;
			}
		}

		Logger::String{"Cant find attribute '",attrname,"'"}.trace("xmlnode");

		return pugi::xml_node();
	}

	void Abstract::Object::set(const pugi::xml_node &node) {

		Udjat::NamedObject::set(node);

	}

	const char * Abstract::Object::get_text(const pugi::xml_node &node, const char *attrname) {

		pugi::xml_node child = find(node,attrname);
		if(!child) {
			return "";
		}

		String value{child};
		value.expand(node,child.attribute("settings-from").as_string("widget-defaults"));

		return value.as_quark();

	}

	bool Abstract::Object::getProperty(const char *key, std::string &value) const {

		if(!strcasecmp(key,"template-dir")) {

#ifdef DEBUG
			value = "./templates";
#else
			value = Udjat::Application::DataDir{"templates"};
#endif // DEBUG
			return true;
		}

		return Udjat::NamedObject::getProperty(key,value);
	}


 }

