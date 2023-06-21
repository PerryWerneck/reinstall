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

 namespace Udjat {

	/// @brief Standard dialog.
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

		constexpr Dialog(const char *t, const char *m, const char *s) : title{t}, message{m}, secondary{s} {
		}

		constexpr Dialog(const char *m, const char *s) : message{m}, secondary{s} {
		}

		/// @brief Run dialog with buttons.
		/// @return The id of selected button.
		virtual int run(const std::vector<Button> &buttons);

		/// @brief Show progress dialog, run background task.
		/// @return Return code of the background task.
		virtual int run(const std::function<int(Progress &progress)> &background_task) const;

	};

 }

