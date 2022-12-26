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

 #include <reinstall/action.h>
 #include <reinstall/iso9660.h>
 #include <reinstall/script.h>
 #include <vector>

 namespace Reinstall {

	class UDJAT_API IsoBuilder : public Reinstall::Action {
	protected:

		const char *system_area = nullptr;
		const char *volume_id = nullptr;
		const char *publisher_id = nullptr;
		const char *data_preparer_id = nullptr;
		const char *application_id = nullptr;
		const char *system_id = nullptr;

		struct {
			const char *boot_image = "/boot/x86_64/loader/isolinux.bin";
            const char *catalog = "/boot/x86_64/loader/boot.cat";
		} eltorito;

		struct {
			const char *boot_image = "/boot/x86_64/efi";
		} efi;

		/// @brief Write ISO image.
		virtual void write(iso9660::Worker &worker) = 0;

	protected:

		/// brief Post scripts.
		std::vector<Script> post_scripts;

		/// @brief Run image post-processing (isohybrid).
		void post(const char *isoname);

	public:
		IsoBuilder(const pugi::xml_node &node);
		virtual ~IsoBuilder();

		/// @brief First step, build image.
		void prepare() override;

	};

 }
