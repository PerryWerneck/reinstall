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
  * @brief Declare libreinstall popup dialogs.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/ui/dialog.h>

 namespace Reinstall {

	/// @brief Action popup
	class Popup : public Udjat::Dialog {
	private:
		bool allow_reboot = false;
		bool allow_quit = false;

	public:
		Popup(const char *name, const Udjat::XML::Node &node) {
			Udjat::Dialog::setup(name,node);
		}

		void setup(const Udjat::XML::Node &node) override;

		void run(const char *error_message, bool allow_close = true) const;
		void run(bool allow_close = true) const;

	};

 }

 using Popup = Reinstall::Popup;

