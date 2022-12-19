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

 int main(int argc, char* argv[]) {

	class MainWindow : public Gtk::Window {
	public:
		MainWindow() {
		  set_title(PACKAGE_STRING);
		  set_default_size(200, 200);
		}
	};


	auto app = Gtk::Application::create("br.com.bb.reinstall");

	MainWindow window;
	window.set_default_size(200, 200);

	return app->run(window);
 }
