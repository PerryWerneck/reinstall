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
  * @brief Brief description of this source.
  */

 #pragma once

 #include <gtkmm.h>
 #include <glibmm/i18n.h>

 #include <udjat/defs.h>
 #include <udjat/tools/application.h>

 namespace Udjat {

	namespace Gtk {

		class Application : public Udjat::Application, public ::Gtk::Application {
		protected:
			Application(const Glib::ustring& application_id);

			inline Application(const char *application_id) : Udjat::Gtk::Application{Glib::ustring{application_id}} {
			}

			int run(const char *definitions = nullptr) override;

		public:
			virtual ~Application();

			static Glib::RefPtr<Application> create(const char *application_id);

			inline int run(int argc, char* argv[]) {
				return Udjat::Application::run(argc,argv);
			}

		};

	}

 }
