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
 #include <reinstall/defs.h>
 #include <reinstall/dialogs/window.h>
 #include <memory>
 #include <functional>

 namespace Reinstall {

	namespace Dialog {

		/// @brief Dialog button.
		class UDJAT_API Button {
		public:
			Button() = default;

			/// @brief Enable button, call activate() on 'true' if not overrided.
			virtual void enable(bool en = true);

			inline void disable() {
				enable(false);
			}

			virtual void set_destructive();
			virtual void set_suggested();

			virtual void activate() = 0;
		};

		/// @brief Dialog for background tasks.
		class UDJAT_API TaskRunner : public Window {
		public:

			TaskRunner();
			virtual ~TaskRunner();

			/// @brief Show Dialog.
			virtual void show();

			/// @brief Update dialog with popup definitions.
			virtual void set(const Popup &popup);

			/// @brief Update dialog with action definitions.
			virtual void set(const Action &action);

			virtual void set_title(const char *text, bool markup = false);
			virtual void set_sub_title(const char *text, bool markup = false);

			/// @brief Button factory.
			virtual std::shared_ptr<Button> ButtonFactory(const char *label, const std::function<void()> &callback);

			/// @brief Run background task (if available).
			/// @param callback task to run.
			/// @param show show dialog after task begin.
			/// @return Task response or negative errno in case of exception.
			/// @retval -1 Unexpected exception while running task.
			virtual int push(const std::function<int()> &callback, bool show = true);
		};

	}

 }

