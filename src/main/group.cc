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
	::Gtk::Box{::Gtk::ORIENTATION_VERTICAL},label{node,"title"}, subtitle{XML::StringFactory(node,"sub-title").c_str()} {

	// Setup group.
	set_name(XML::StringFactory(node,"name").c_str());
	get_style_context()->add_class("group-box");

	set_hexpand(true);
	set_vexpand(false);
	set_halign(ALIGN_FILL);

	set_vexpand(false);
	set_valign(ALIGN_START);

	// The title box
	title.set_halign(ALIGN_START);
	title.set_hexpand(false);
	title.set_vexpand(false);
	title.get_style_context()->add_class("group-title-box");

	// The title text.
	label.set_hexpand(false);
	label.set_vexpand(false);
	label.get_style_context()->add_class("group-title");
	title.pack_start(label);

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
			title.pack_start(linkbutton);
		}
	}

	pack_start(title);

	// The group sub-title
	if(subtitle) {
		subtitle.set_hexpand(true);
		subtitle.set_vexpand(false);
		subtitle.get_style_context()->add_class("group-subtitle");
		pack_start(subtitle);
	}

 }

  void MainWindow::Group::push_back(::Gtk::Widget &child) {

 	child.set_halign(ALIGN_FILL);
	child.set_hexpand(true);
	child.set_vexpand(false);
	child.get_style_context()->add_class("group-item");

	pack_start(child);
	show_all();

  }


