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
  * @brief Implements action scripts.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/script.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/intl.h>
 #include <libreinstall/action.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	Action::Script::Script(const Udjat::XML::Node &node)
		: Udjat::Script{node},type{(Action::Script::Type) Udjat::String{node,"type","post"}.select("pre","post",NULL)} {

		if(type < 0) {
			throw runtime_error(_("Invalid 'type' attribute"));
		}

	}

 }

