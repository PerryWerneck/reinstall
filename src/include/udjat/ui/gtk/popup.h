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
  * @brief Describe GTK Progress dialog.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <gtkmm.h>
 #include <glibmm/i18n.h>
 #include <udjat/ui/dialogs/popup.h>
 #include <udjat/ui/gtk/label.h>

 namespace Udjat {

	namespace Gtk {

		class Popup : public ::Gtk::MessageDialog, public Udjat::Dialog::Popup {
		private:
			int response = -1;

		protected:
			void on_response (int response_id) override;

		public:
			Popup(::Gtk::Window &parent, const char *message = "", const char *secondary = "");
			Popup(::Gtk::Window &parent, const Udjat::Dialog &dialog, const std::vector<Udjat::Dialog::Button> &buttons);

			virtual ~Popup();

			void set(const Udjat::Dialog &dialog) override;
			void enable(int id, bool enabled = true) override;
			bool set_label(int id, const char *label) override;
			int run();
			int run(const std::function<int(Udjat::Dialog::Popup &popup)> &task);

		};

	}

 }
