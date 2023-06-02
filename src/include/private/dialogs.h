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
 #include <reinstall/dialogs/popup.h>
 #include <reinstall/dialogs/progress.h>
 #include <reinstall/dialogs/taskrunner.h>
 #include <reinstall/object.h>

 namespace Dialog {

	/// @brief Popup dialog.
	class Popup : public Gtk::MessageDialog  {
	private:
		Gtk::LinkButton url;

	public:
		Popup(Gtk::Window &parent, const Reinstall::Abstract::Object &object, const Reinstall::Dialog::Popup &settings, Gtk::MessageType = Gtk::MESSAGE_INFO, Gtk::ButtonsType buttons = Gtk::BUTTONS_OK);

		void set(const Reinstall::Dialog::Popup &settings);

	};

	/// @brief TaskRunner dialog
	class TaskRunner : public Gtk::MessageDialog, public Reinstall::Dialog::TaskRunner  {
	public:
		TaskRunner(Gtk::Window &parent, const char *message, bool markup = false);
		virtual ~TaskRunner();

		void show() override;

		// void allow_continue(bool allowed) override;
		void set_title(const char *text, bool markup) override;
		void set_sub_title(const char *text, bool markup) override;

		void set(const Reinstall::Action &action) override;

		int push(const std::function<int()> &callback, bool show = true) override;

		std::shared_ptr<Reinstall::Dialog::Button> ButtonFactory(const char *label, const std::function<void()> &callback) override;

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
			Glib::RefPtr<Glib::TimeoutSource> source;
			unsigned int idle = (unsigned int) -1;
		} timer;

		struct {
			Label title{ "" };
			Label subtitle{ "" };
			Label left{ "", Gtk::ALIGN_START };
			Label right{ "", Gtk::ALIGN_END };

			Gtk::Grid header;
			Gtk::Image icon;
			Gtk::ProgressBar progress;
			Gtk::Box footer{Gtk::ORIENTATION_HORIZONTAL};

		} widgets;

		struct {
			std::string title;
			std::string subtitle;
		} text;

	public:
		Progress();
		virtual ~Progress();

		void set_parent(Gtk::Window &window);

		inline Label & title() {
			return widgets.title;
		}

		inline Gtk::Image & icon() {
			return widgets.icon;
		}

		void footer(bool enable = true);

		void dismiss(int response_id = -1);

		// Interface with libreinstall.
		void show() override;
		void hide() override;
		void pulse() override;
		void set_title(const char *title) override;
		void set_sub_title(const char *title) override;
		void set_step(const char *step) override;
		void set_icon_name(const char *icon_name) override;
		void set(const Reinstall::Abstract::Object &object) override;
		void set_url(const char *url) override;

		const char * get_title() const override;
		const char * get_sub_title() const override;

	};

 }
