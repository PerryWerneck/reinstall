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
 #include <udjat/defs.h>
 #include <stddef.h>
 #include <cstdint>
 #include <libreinstall/dialogs/window.h>
 #include <string>
 #include <functional>

 namespace Reinstall {

	namespace Dialog {

		/// @brief Proxy for progress dialog.
		class UDJAT_API Progress : public Window {
		protected:
			struct {
				bool changed = false;
				double current = 0;
				double total = 0;
			} values;

			enum : uint8_t {
				HIDE_URL,
				SHOW_URL_ON_SUBTITLE,
				SHOW_URL_ON_PROGRESS_BAR
			} url_mode = SHOW_URL_ON_PROGRESS_BAR;

		public:
			Progress(const Progress &) = delete;
			Progress(const Progress *) = delete;

			Progress() : Window() {
			}

			~Progress();

			static Progress & getInstance();

			// virtual void set(const Action &action);
			virtual void set_title(const char *title);
			virtual void set_sub_title(const char *subtitle);
			virtual void set_step(const char *step);
			virtual void set_icon_name(const char *icon_name);
			virtual void pulse();

			virtual const char * get_title() const;
			virtual const char * get_sub_title() const;

			// Extends Udjat::Protocol::Watcher.
			virtual void set_url(const char *url);
			virtual void set_progress(double current, double total);

			/// @brief Update count of downloaded files.
			virtual void set_count(size_t current, size_t total);

			/// @brief Set progress title (not the window title).
			inline void set_title(const std::string &message) {
				set_title(message.c_str());
			}

			int run(const char *name, const std::function<void()> &background_task);

		};

	}

 }

