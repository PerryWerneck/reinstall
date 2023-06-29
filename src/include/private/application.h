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

 #pragma once

 #include <config.h>
 #include <udjat/defs.h>
 #include <gtkmm.h>
 #include <glibmm/i18n.h>
 #include <udjat/ui/gtk/application.h>
 #include <private/mainwindow.h>

 /// @brief The application object.
 class UDJAT_PRIVATE Application : public Udjat::Gtk::Application {
 private:

	MainWindow *window = nullptr;

	Application(const char *application_id) : Udjat::Gtk::Application{application_id} {
	}

 protected:

	void on_startup() override;
	void on_activate() override;

	int init(const char *) override;

 public:
	static Glib::RefPtr<Application> create();

 };
