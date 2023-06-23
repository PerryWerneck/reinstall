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

 /**
  * @brief Implements group widget.
  */

 #include <config.h>

 #include <config.h>
 #include <udjat/defs.h>
 #include <gtkmm.h>
 #include <glibmm/i18n.h>
 #include <private/mainwindow.h>
 #include <udjat/tools/xml.h>

 #include <udjat/ui/gtk/label.h>
 #include <udjat/ui/gtk/icon.h>


 using namespace Udjat;
 using namespace std;
 using namespace ::Gtk;

 MainWindow::Group::Group(const XML::Node &node) :

	label{node,"title"}, body{"sub-title"} {

	set_name(XML::StringFactory(node,"name").c_str());

	// Setup group.
	set_hexpand(true);
	set_halign(ALIGN_FILL);

	set_vexpand(false);
	set_valign(ALIGN_START);

	label.set_hexpand(true);
	body.set_hexpand(true);
	label.set_vexpand(false);
	body.set_vexpand(false);

	get_style_context()->add_class("group-box");
	label.get_style_context()->add_class("group-title");

	attach(label,1,0,1,1);
	if(body) {
		body.get_style_context()->add_class("group-subtitle");
		attach(body,1,1,2,1);
	}

	actions.get_style_context()->add_class("group-actions");
	attach(actions,1,2,2,1);
	show_all();

 };
