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
 #include <reinstall/action.h>

 namespace Reinstall {

	namespace Dialog {

		class UDJAT_API Window {
		private:
			static Window * current;
			Window *parent;

		public:
			Window();
			virtual ~Window();

			static Window & getInstance();

			virtual void show();
			virtual void hide();

			virtual void set(const Abstract::Object &object);

		};

		/// @brief Dialog for background tasks.
		class UDJAT_API TaskRunner : public Window {
		protected:
			bool is_enabled = true;

		public:
			TaskRunner();
			virtual ~TaskRunner();

			inline bool enabled() const noexcept {
				return is_enabled;
			}

			inline void disable() noexcept {
				is_enabled = false;
			}

			/// @brief Allow 'continue' button.
			virtual void allow_continue(bool allowed);

			virtual void set_title(const char *markup);
			virtual void set_sub_title(const char *markup);

			virtual void add_button(const char *label, const std::function<void()> &callback);

			virtual int push(const std::function<int()> &callback);
		};

		/// @brief Proxy for progress dialog.
		class UDJAT_API Progress : public Window {
		public:
			Progress(const Progress &) = delete;
			Progress(const Progress *) = delete;

			Progress() : Window() {
			}

			~Progress();

			static Progress & getInstance();

			virtual void set(const Action &action);
			virtual void set_title(const char *title);
			virtual void set_sub_title(const char *subtitle);
			virtual void set_step(const char *step);
			virtual void set_icon_name(const char *icon_name);

			// Extends Udjat::Protocol::Watcher.
			virtual void set_url(const char *url);
			virtual void set_progress(double current, double total);

			/// @brief Update count of downloaded files.
			virtual void set_count(size_t current, size_t total);

			/// @brief Set progress title (not the window title).
			inline void set_title(const std::string &message) {
				set_title(message.c_str());
			}

		};

	}

 }

