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

 #include <gtkmm.h>
 #include <glibmm/i18n.h>

 namespace Dialog {

	/// @brief Standar progress dialog.
	class Progress : public Gtk::Dialog {
	private:

		struct {
			unsigned int idle = (unsigned int) -1;
			sigc::connection connection;
		} timer;

		struct {
			Gtk::Label title{ "title", Gtk::ALIGN_START };
			Gtk::Label action{ "action", Gtk::ALIGN_START };
			Gtk::Label message{ "message", Gtk::ALIGN_CENTER };
			Gtk::Label step{ "step", Gtk::ALIGN_END };
			Gtk::Image icon;
			Gtk::ProgressBar progress;
			Gtk::Box footer{Gtk::ORIENTATION_HORIZONTAL};
		} widgets;

		bool on_timeout(int timer_number);

	public:
		Progress();

	};

 }
