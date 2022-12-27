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
 #include <private/widgets.h>

 using namespace Gtk;
 using namespace std;

 namespace Widget {

	 Group::Group(const pugi::xml_node &node) : label{node,"title"}, body{node,"sub-title"}, icon{node,"icon",Gtk::ICON_SIZE_DND} {

		set_hexpand(true);
		set_vexpand(false);
		set_valign(ALIGN_START);
		set_halign(ALIGN_START);

		get_style_context()->add_class("group-box");
		label.get_style_context()->add_class("group-title");

		/*
		if(icon) {
			icon.get_style_context()->add_class("action-icon");
			icon.set_valign(ALIGN_CENTER);
			icon.set_halign(ALIGN_CENTER);
			attach(icon,0,0,1,2);
		}
		*/

		attach(label,1,0,1,1);
		if(body) {
			body.get_style_context()->add_class("group-subtitle");
			attach(body,1,1,2,1);
		}

		set(node);

		show_all();

	 }

	std::string Group::get_label() const {
		return label.get_text().c_str();
	}

 }
