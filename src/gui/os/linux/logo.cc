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
 #include <private/mainwindow.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/logger.h>
 #include <iostream>
 #include <unistd.h>

 static std::string find_logo() {

	std::string basename{g_application_get_application_id(g_application_get_default())};

	std::string filenames[] {
		(std::string{"/usr/share/pixmaps/"} + basename + ".svg"),
		(std::string{"/usr/share/pixmaps/"} + basename + ".png"),
		(std::string{"/usr/share/icons/"} + basename + ".svg")
	};

	for(std::string &filename : filenames) {

		if(access(filename.c_str(),R_OK) == 0) {
			Udjat::Logger::String("Using logo from '",filename.c_str(),"'").trace("MainWindow");
			return filename;
		}

	}

	std::cout << "------------------------------------" << std::endl;
	Udjat::Logger::String("Cant find '",basename,".[svg|png]', using system default").trace("MainWindow");

	return Udjat::Config::Value<std::string>{"MainWindow","sidebar-logo","/usr/share/pixmaps/distribution-logos/square-hicolor.svg"};
 }

 MainWindow::Logo::Logo() : Gtk::Image{find_logo()} {

	// https://developer-old.gnome.org/gtkmm/stable/classGtk_1_1Image.html
	set_hexpand(true);
	set_vexpand(false);
	get_style_context()->add_class("logo");

 }

