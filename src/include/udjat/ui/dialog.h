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
	protected:

		/// @brief The dialog icon.
		const char *icon = "";

		/// @brief The dialog title.
		const char *title = "";

		/// @brief The dialog primary message.
		const char *message = "";

		/// @brief The dialog secondary message.
		const char *secondary = "";

		/// @brief Setup dialog from node.
		void setup(const XML::Node &node);

		/// @brief Scan for tag "<dialog name='name'>" and setup dialog from it.
		/// @return true if the dialog was set.
		bool setup(const char *name, const XML::Node &node);

	public:

		class Progress;
		class Button;

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

		/// @brief Run dialog with buttons.
		/// @param buttons the dialog buttons.
		/// @return The id of selected button.
		int run(const std::vector<Button> &buttons);

		/// @brief Build progress dialog, run background task.
		/// @param task the background task to run.
		/// @return Return code of the background task.
		int run(const std::function<int(Progress &progress)> &task) const;

	};

 }

