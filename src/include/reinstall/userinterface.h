/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2022 Perry Werneck <perry.werneck@gmail.com>
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
 #include <reinstall/group.h>
 #include <libreinstall/dialogs/progress.h>
 #include <memory>

 namespace Reinstall {

	class UDJAT_API UserInterface {
	private:
		static UserInterface *instance;

	public:
		UserInterface();
		virtual ~UserInterface();

		static UserInterface & getInstance();

		static void textmode();

		/// @brief Get filename (gui thread).
		virtual std::string FilenameFactory(const char *title, const char *label_text, const char *apply, const char *filename, bool save);

		/// @brief Construct an action button.
		/// @param node The xml node with action definitions.
		/// @param icon_name The default icon name.
		// virtual std::shared_ptr<Abstract::Object> ActionFactory(const pugi::xml_node &node, const char *icon_name = "");

		/// @brief Construct a group box.
		// virtual std::shared_ptr<Abstract::Group> GroupFactory(const pugi::xml_node &node);

		/// @brief Construct a wait for task dialog.
		// virtual std::shared_ptr<Dialog::TaskRunner> TaskRunnerFactory( const char *message, bool markup = false);

	};


 }
