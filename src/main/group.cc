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
 #include <udjat/factory.h>

 #include <udjat/ui/gtk/label.h>
 #include <udjat/ui/gtk/icon.h>


 using namespace Udjat;
 using namespace std;
 using namespace ::Gtk;

 MainWindow::Group::Group(const XML::Node &node) :
	label{node,"title"}, body{XML::StringFactory(node,"sub-title").c_str()} {

	set_name(XML::StringFactory(node,"name").c_str());

	// Setup group.
	set_hexpand(true);
	set_halign(ALIGN_FILL);

	set_vexpand(false);
	set_valign(ALIGN_START);

	// The group title.
	label.set_hexpand(false);
	label.set_vexpand(false);
	get_style_context()->add_class("group-box");
	label.get_style_context()->add_class("group-title");
	attach(label,1,0,1,1);

	// Check help link.
	{
		auto help = XML::StringFactory(node,"help");
		if(!help.empty()) {
			// https://developer-old.gnome.org/gtkmm/stable/classGtk_1_1LinkButton.html
			linkbutton.get_style_context()->add_class("group-link");

			linkbutton.set_uri(help);
			linkbutton.set_hexpand(false);
			linkbutton.set_vexpand(false);
			linkbutton.set_halign(::Gtk::ALIGN_START);

			// https://specifications.freedesktop.org/icon-naming-spec/latest/ar01s04.html
			linkbutton.set_image_from_icon_name("help-faq");
			attach_next_to(linkbutton,label,::Gtk::POS_RIGHT);
		}
	}


	// The group sub-title
	if(body) {
		body.set_hexpand(true);
		body.set_vexpand(false);
		body.get_style_context()->add_class("group-subtitle");
		attach(body,1,1,3,1);
	}

	// Load children.
	for(auto child : node) {

		Factory::for_each(child.name(),[this,&child](Factory &factory) {

			try {

				return factory.generic(child);

			} catch(const std::exception &e) {

				factory.error() << "Cant parse node <" << child.name() << ">: " << e.what() << endl;

			} catch(...) {

				factory.error() << "Cant parse node <" << child.name() << ">: Unexpected error" << endl;

			}

			return false;

		});

	}

	// The box for group actions.
	actions.get_style_context()->add_class("group-actions");
	attach(actions,1,2,2,1);
	show_all();

 };
