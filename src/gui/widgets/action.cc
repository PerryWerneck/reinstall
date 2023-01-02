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
 #include <udjat/tools/quark.h>

 using namespace Gtk;
 using namespace std;
 using namespace Udjat;

 namespace Widget {

	Gtk::RadioButton::Group Widget::Action::group;

	Action::Action(const pugi::xml_node &node, const char *icon_name)
		: Gtk::RadioButton{group}, label{node,"title"}, body{node,"sub-title"}, icon{node,Gtk::ICON_SIZE_DND,icon_name,true} {

		const char *group = node.attribute("settings-from").as_string("action-defaults");

		set_hexpand(true);
		set_vexpand(false);
		set_valign(ALIGN_START);
		set_halign(ALIGN_FILL);

		set_inconsistent();
		set_mode(false);

		get_style_context()->add_class("action-button");
		grid.get_style_context()->add_class("action-container");
		label.get_style_context()->add_class("action-title");

		grid.attach(label,1,0,1,1);
		if(body) {
			body.get_style_context()->add_class("action-subtitle");
			grid.attach(body,1,1,2,1);
		}

		grid.show_all();

		if(icon && Udjat::Object::getAttribute(node,group,"show-icon",true)) {
			icon.get_style_context()->add_class("action-icon");
			grid.attach(icon,0,0,1,2);
			icon.show();
		} else {
			icon.hide();
		}

		add(grid);

		set(node);

		if(get_active()) {
			get_style_context()->add_class("action-active");
		} else {
			get_style_context()->add_class("action-inactive");
		}

		signal_toggled().connect([&]() {

			if(get_active()) {
				get_style_context()->remove_class("action-inactive");
				get_style_context()->add_class("action-active");
			} else {
				get_style_context()->remove_class("action-active");
				get_style_context()->add_class("action-inactive");
			}

		});

		show();
	}

	std::string Action::get_label() const {
		return label.get_text().c_str();
	}

 }
