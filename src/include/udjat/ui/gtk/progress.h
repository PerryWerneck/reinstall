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
 #include <udjat/ui/dialogs/progress.h>
 #include <udjat/ui/gtk/label.h>

 namespace Udjat {

	namespace Gtk {

		class Progress : public ::Gtk::Dialog, public Udjat::Dialog::Progress {
		private:

			struct {
				Glib::RefPtr<Glib::TimeoutSource> source;
				unsigned int idle = (unsigned int) -1;
			} timer;

			struct {
				Label title{ "" };
				Label message{ "" };
				Label left{ "", ::Gtk::ALIGN_START };
				Label right{ "", ::Gtk::ALIGN_END };

				::Gtk::Grid header;
				::Gtk::Image icon;
				::Gtk::ProgressBar progress;
				::Gtk::Box footer{::Gtk::ORIENTATION_HORIZONTAL};

			} widgets;

			struct {
				std::string title;
				std::string message;
				std::string url;
			} text;

			struct {
				bool changed = false;
				double current = 0;
				double total = 0;
			} values;

		public:
			Progress(::Gtk::Window &parent, const Udjat::Dialog &dialog);

			virtual ~Progress();

			int run(const std::function<int(Udjat::Dialog::Progress &progress)> &task);

			void title(const char *title) override;

			void message(const char *message) override;

			std::string message() const override;

			void icon(const char *icon_name) override;

			void url(const char *url) override;

			void pulse() override;

			void progress(double current, double total) override;

			void count(size_t current, size_t total) override;

		};

	}

 }
