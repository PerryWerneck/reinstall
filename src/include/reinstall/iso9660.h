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
 #include <reinstall/worker.h>

 typedef struct Iso_Image IsoImage;
 typedef struct iso_write_opts IsoWriteOpts;

 namespace Reinstall {

	namespace iso9660 {

		/// @brief Worker for building an iso image.
		class UDJAT_API Worker : public Reinstall::Worker {
		private:
			IsoImage *image = nullptr;
			IsoWriteOpts *opts;

		protected:
			void push_back(Source &source) override;

		public:

			Worker();
			~Worker();

			/// @brief Save image to 'fd'.
			void save(int fd);

			/// @brief Save image to file.
			void save(const char *filename);

			void set_system_area(const char *path = nullptr);
			void set_volume_id(const char *volume_id);
			void set_publisher_id(const char *publisher_id = nullptr);
			void set_data_preparer_id(const char *data_preparer_id = nullptr);
			void set_application_id(const char *application_id);
			void set_system_id(const char *system_id);
			void set_iso_level(int level = 3);
			void set_rockridge(int rockridge = 1);
			void set_joliet(int joliet = 1);
			void set_allow_deep_paths(int deep_paths = 1);
			void set_el_torito_boot_image(const char *boot_image, const char *catalog, const char *id = nullptr);
			void set_efi_boot_image(const char *isopath, bool like_iso_hybrid = true);
			void add_boot_image(const char *isopath, uint8_t id);

		};

	}

 }
