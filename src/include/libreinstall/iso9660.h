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
  * @brief Declare iso9660 abstract classes.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>

 namespace iso9660 {

	class UDJAT_API Settings {
	public:

		const char *system_area = nullptr;
		const char *volume_id = nullptr;
		const char *publisher_id = nullptr;
		const char *data_preparer_id = nullptr;
		const char *application_id = nullptr;
		const char *system_id = nullptr;

		struct Boot {

			const char *catalog = "/boot/x86_64/loader/boot.cat";

			/// @brief el-torito boot settings.
			struct ElTorito {

				constexpr ElTorito() = default;
				ElTorito(const Udjat::XML::Node &node);

				bool enabled = true;
				const char *image = "/boot/x86_64/loader/isolinux.bin";

				inline operator bool() const {
					return enabled;
				}

			} eltorito;

			struct Efi {

				constexpr Efi() = default;
				Efi(const Udjat::XML::Node &node);

				bool enabled = true;

				inline operator bool() const {
					return enabled;
				}

			} efi;

			constexpr Boot() = default;
			Boot(const Udjat::XML::Node &node);

		} boot;

		constexpr Settings() = default;
		Settings(const Udjat::XML::Node &node);

	};


 }
