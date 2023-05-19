/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <pugixml.hpp>
 #include <udjat/tools/intl.h>
 #include <reinstall/dialogs.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

 	void Dialog::Popup::set(const pugi::xml_node &node) {

		const char *group = node.attribute("settings-from").as_string("popup-defaults");

		destructive = Udjat::Object::getAttribute(node,group,"destructive",false);

		message = Udjat::Object::getAttribute(node,group,"message","");

		url.link = Udjat::Object::getAttribute(node,group,"url","");
		url.label = Udjat::Object::getAttribute(node,group,"url-label",_("More info"));

		{
			Udjat::String text{node.child_value()};
			text.expand(node,group);
			text.strip();
			if(!text.empty()) {
				secondary = text.as_quark();
			}
		}

	}

 }
