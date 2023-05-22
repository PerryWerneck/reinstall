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
 #include <pugixml.hpp>

 namespace Reinstall {

	namespace Dialog {

		/// @brief Dialog box from XML.
		class UDJAT_API Popup {
		public:
			const char *message = "";
			const char *secondary = "";

			bool destructive = false;
			bool markup = false;

			struct {
				const char *link = "";
				const char *label = "";

				inline operator bool() const {
					return (link && *link);
				}

			} url;

			Popup() = default;

			void set(const pugi::xml_node &node);

			inline operator bool() const noexcept {
				return (message && *message);
			}

			inline bool has_secondary() const noexcept {
				return secondary && *secondary;
			}


		};

	}

 }

