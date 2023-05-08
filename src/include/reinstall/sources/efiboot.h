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

 #pragma once

 #include <udjat/defs.h>
 #include <reinstall/source.h>
 #include <pugixml.hpp>
 #include <udjat/tools/object.h>

 namespace Reinstall {

	/// @brief The EFI Boot Image;
	class UDJAT_API EFIBootImage : Udjat::NamedObject {
	private:

		struct {
			bool enabled = true;

			/// @brief Path for EFI boot inside image.
			const char *path = "/boot/x86_64/efi";

		} options;

	public:
		EFIBootImage(const pugi::xml_node &node);

		inline bool enabled() const noexcept {
			return options.enabled;
		}

		inline const char * path() const noexcept {
			return options.path;
		}

	};

 }
