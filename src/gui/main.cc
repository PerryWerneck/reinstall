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
 #include <gtkmm.h>
 #include <glibmm/i18n.h>
 #include <iostream>

 using namespace Gtk;
 using namespace std;

 int main(int argc, char* argv[]) {

	class MainWindow : public Window {
	private:
		Label title{ _( "Select option" ) };
		Gtk::Box hbox, vbox;
		Gtk::ScrolledWindow view;
		Gtk::ButtonBox bbox;
		Gtk::Button apply{_("Apply")};
		Gtk::Button cancel{_("Cancel")};

	public:
		MainWindow() {

			set_title(PACKAGE_STRING);
			set_default_size(600, 400);

			// https://developer-old.gnome.org/gtkmm-tutorial/stable/sec-clipboard-examples.html.en
			// http://transit.iut2.upmf-grenoble.fr/doc/gtkmm-3.0/tutorial/html/sec-helloworld.html

			vbox.set_hexpand(true);
			vbox.set_vexpand(true);
			vbox.set_orientation(ORIENTATION_VERTICAL);
			vbox.set_border_width(6);

			vbox.add(title);

			view.set_hexpand(true);
			view.set_vexpand(true);
			vbox.add(view);

			bbox.add(cancel);
			bbox.add(apply);
			bbox.set_layout(BUTTONBOX_END);
			bbox.set_hexpand(true);
			vbox.add(bbox);

			/*
			grid.set_hexpand(true);
			grid.set_vexpand(true);
			grid.set_row_homogeneous(false);
			grid.set_column_homogeneous(false);

			grid.attach(title,1,0,1,1);		// widget, left, top, width, height


			bbox.add(cancel);
			bbox.add(apply);
			bbox.set_border_width(6);
			bbox.set_hexpand(true);
			grid.attach(bbox,1,2,1,1);

			*/

			hbox.set_hexpand(true);
			hbox.set_vexpand(true);
			hbox.add(vbox);
			add(hbox);
			show_all();
		}
	};

	auto app = Application::create("br.com.bb.reinstall");

	MainWindow window;
	window.set_default_size(200, 200);

	return app->run(window);
 }
