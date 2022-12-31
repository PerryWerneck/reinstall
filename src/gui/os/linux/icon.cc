/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2022 Perry Werneck <perry.werneck@gmail.com>
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
 #include <private/widgets.h>
 #include <reinstall/action.h>
 #include <cstring>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/object.h>

 using namespace Udjat;

 namespace Widget {

	Icon::Icon(const pugi::xml_node &node, const Gtk::IconSize iconsize, const char *def, bool symbolic) {

		const char *name = Udjat::Object::getAttribute(node, "icon", false).as_string(def);

		// https://developer-old.gnome.org/gtkmm/stable/classGtk_1_1Image.html

		if(name && *name && strcasecmp(name,"none")) {

			if(symbolic) {
				set_from_icon_name((std::string{name} + "-symbolic").c_str(), iconsize);
			} else {
				set_from_icon_name(name, iconsize);
			}

			show();
		} else {
			hide();
		}

	}

	Icon::operator bool() const {
		return get_storage_type() != Gtk::IMAGE_EMPTY;
	}

 }

