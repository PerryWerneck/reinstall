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
 #include <reinstall/dialogs.h>
 #include <reinstall/object.h>

 namespace Dialog {

	/// @brief Popup dialog.
	class Popup : public Gtk::MessageDialog  {
	public:
		Popup(Gtk::Window &parent, const Reinstall::Abstract::Object &object, const Reinstall::Abstract::Popup &settings, Gtk::MessageType = Gtk::MESSAGE_INFO, Gtk::ButtonsType buttons = Gtk::BUTTONS_OK);

	};

	/// @brief Standard progress dialog.
	class Progress : public Reinstall::Dialog::Progress, public Gtk::Dialog {
	public:
		class Label : public Gtk::Label {
		public:
			Label(const char *text, Gtk::Align halign = Gtk::ALIGN_START, Gtk::Align valign=Gtk::ALIGN_CENTER) : Gtk::Label(text,halign,valign) {
			}

			inline Label & operator = (const char *value) {
				set_text(value);
				return *this;
			}

		};

	private:

		struct {
			unsigned int idle = (unsigned int) -1;
			sigc::connection connection;
		} timer;

		struct {
			Label title{ "" };
			Label action{ "", Gtk::ALIGN_START };
			Label message{ "", Gtk::ALIGN_CENTER };
			Label step{ "", Gtk::ALIGN_END };

			Gtk::Image icon;
			Gtk::ProgressBar progress;
			Gtk::Box footer{Gtk::ORIENTATION_HORIZONTAL};

		} widgets;

		bool on_timeout(int timer_number);
		bool on_dismiss(int response_id);

	public:
		Progress();

		void set_parent(Gtk::Window &window);

		inline Label & sub_title() {
			return widgets.title;
		}

		inline Label & action() {
			return widgets.action;
		}

		inline Label & message() {
			return widgets.message;
		}

		inline Label & step() {
			return widgets.step;
		}

		inline Gtk::Image & icon() {
			return widgets.icon;
		}

		void footer(bool enable = true);

		void dismiss(int response_id = -1);

		// Interface with libreinstall.
		void show() override;
		void hide() override;
		void set(const char *message) override;
		void count(size_t count, size_t total) override;
		void update(double current, double total) override;
		void set(const Reinstall::Abstract::Object &object) override;

	};

 }
