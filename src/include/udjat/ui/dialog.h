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
  * @brief Declare the abstract dialog.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <functional>
 #include <vector>
 #include <memory>

 namespace Udjat {

	/// @brief Generic dialog box.
	class UDJAT_API Dialog {
	public:

		class Progress;
		class Popup;

		class UDJAT_API Button {
		public:

			/// @brief The button id.
			int id = -1;

			/// @brief The button label.
			const char * label;

			enum Style : uint8_t {
				Standard,
				Recommended,
				Destructive
			} style = Standard;

			constexpr Button(int i, const char *l, const Style s = Standard) : id{i}, label{l}, style{s} {
			}

		};

		class UDJAT_API Controller {
		private:
			static Controller *cntrl;

		public:
			Controller();
			~Controller();

			static Controller & instance();

			/// @brief Quit application.
			virtual void quit(int rc = 0);

			/// @brief Run confirmation dialog.
			virtual bool confirm(const Dialog &dialog, const char *yes = nullptr, const char *no = nullptr);

			/// @brief Run dialog with buttons.
			/// @param buttons the dialog buttons.
			/// @return The id of selected button.
			virtual int run(const Dialog &dialog, const std::vector<Button> &buttons);

			/// @brief Build default progress dialog, run background task.
			/// @param task the background task to run.
			/// @return Return code of the background task.
			int run(const std::function<int(Progress &progress)> &task);

			/// @brief Build progress dialog, run background task.
			/// @param task the background task to run.
			/// @return Return code of the background task.
			virtual int run(const Dialog &dialog, const std::function<int(Progress &progress)> &task);

			/// @brief Build popup dialog, run background task.
			/// @param buttons the dialog buttons.
			/// @param task the background task to run.
			/// @return The id of selected button.
			virtual int run(const Dialog &dialog, const std::function<int(Popup &popup)> &task, const std::vector<Button> &buttons);

		};

		/// @brief The dialog icon.
		const char *icon = "";

		/// @brief The dialog title.
		const char *title = "";

		/// @brief The dialog primary message.
		const char *message = "";

		/// @brief The dialog secondary message.
		const char *secondary = "";

		/// @brief Setup dialog from node.
		virtual void setup(const XML::Node &node);

		/// @brief Scan for tag "<dialog name='name'>" and setup dialog from it.
		/// @return true if the dialog was set.
		bool setup(const char *name, const XML::Node &node);

		constexpr Dialog() {
		}

		constexpr Dialog(const char *t, const char *m, const char *s) : title{t}, message{m}, secondary{s} {
		}

		constexpr Dialog(const char *m, const char *s) : message{m}, secondary{s} {
		}

		Dialog(const XML::Node &node) {
			setup(node);
		}

		Dialog(const char *name, const XML::Node &node) {
			setup(name,node);
		}

		inline operator bool() const {
			return (message && *message);
		}

		/// @brief Run and yes/no dialog.
		bool confirm(const char *yes = nullptr, const char *no = nullptr) const;

		/// @brief Run dialog with buttons.
		/// @param buttons the dialog buttons.
		/// @return The id of selected button.
		int run(const std::vector<Button> &buttons) const;

		/// @brief Run dialog with buttons.
		/// @param buttons the dialog buttons.
		/// @param task the background task to run.
		/// @return The id of selected button.
		int run(const std::function<int(Popup &popup)> &task, const std::vector<Button> &buttons) const;

		/// @brief run background task.
		/// @param task the background task to run.
		/// @return Return code of the background task.
		int run(const std::function<int(Progress &progress)> &task) const;

	};

 }

