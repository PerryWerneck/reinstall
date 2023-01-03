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

 #include <pugixml.hpp>
 #include <udjat/defs.h>
 #include <udjat/tools/object.h>
 #include <cstring>
 #include <string>

 namespace Reinstall {

	namespace Abstract {

		class UDJAT_API Object : public Udjat::NamedObject {
		public:

			static const char * get_text(const pugi::xml_node &node, const char *attrname);
			static pugi::xml_node find(const pugi::xml_node &node, const char *attrname);

			Object() = default;

			virtual void set(const pugi::xml_node &node);

			virtual std::string get_label() const = 0;

		};

	}

 }

 namespace std {

	inline std::string to_string(const Reinstall::Abstract::Object &object) {
		return object.get_label();
	}

	inline ostream& operator<< (ostream& os, const Reinstall::Abstract::Object &object) {
			return os << object.get_label();
	}

 }

