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

 #include <config.h>
 #include <private/widgets.h>

 using namespace Gtk;
 using namespace std;

 namespace Widget {

	Gtk::RadioButton::Group Widget::Action::group;

	Action::Action(std::shared_ptr<Reinstall::Action> action) : Gtk::RadioButton(group) {

		set_hexpand(true);
		set_vexpand(false);
		set_valign(ALIGN_START);
		set_halign(ALIGN_START);

		get_style_context()->add_class("action-button");
		layout.hbox.get_style_context()->add_class("action-container");

		action->title.get_style_context()->add_class("action-title");

		layout.hbox.pack_start(action->title,false,false,0);

		if(action->subtitle) {
			action->subtitle.get_style_context()->add_class("action-subtitle");
			layout.hbox.pack_start(action->subtitle,false,false,0);
		}

		add(layout.hbox);
		show_all();
	}


 }
