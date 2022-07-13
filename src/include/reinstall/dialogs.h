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

			static Window * getInstance();

			virtual void show();
			virtual void hide();

			virtual void set(const Object &object);

		};

		/// @brief Proxy for progress dialog.
		class UDJAT_API Progress : public Window {
		private:

			// The real progress dialog.
			Progress * worker = nullptr;

		public:
			Progress(const Progress &) = delete;
			Progress(const Progress *) = delete;

			Progress(const Action &action);
			~Progress();

			static Progress & getInstance();

			virtual void set(const char *message);

			inline void set(const std::string &message) {
				set(message.c_str());
			}

			/// @brief Update counters.
			virtual void count(size_t count, size_t total);

			/// @brief Update current operation.
			virtual void update(double current, double total);

		};

	}

 }
