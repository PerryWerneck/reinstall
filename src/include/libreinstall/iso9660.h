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
 #include <libreinstall/writer.h>

 #define LIBISOFS_WITHOUT_LIBBURN
 #include <libisofs/libisofs.h>

 typedef struct Iso_Image IsoImage;
 typedef struct iso_write_opts IsoWriteOpts;

 namespace iso9660 {

	/// @brief ISO-9660 build settings.
	class UDJAT_API Settings {
	public:

		const char *name = nullptr;
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
				bool isohybrid = true;

				const char *image = "/boot/x86_64/loader/isolinux.bin";
				const char *id = "";

				inline operator bool() const {
					return enabled;
				}

			} eltorito;

			struct Efi {

				const char *image = "/boot/x86_64/efi";

				constexpr Efi() = default;
				Efi(const Udjat::XML::Node &node);

				bool enabled = true;
				bool isohybrid = true;

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

	/// @brief ISO-9660 disk image.
	class UDJAT_API Image {
	protected:
		IsoImage *image = nullptr;
		IsoWriteOpts *opts;

	public:

		/// @brief Create a new empty image.
		/// @param name Name of the image. This will be used as volset_id and volume_id.
		Image(const char *name = "");
		~Image();

		/// @brief Write image to device.
		/// @param writer The device writer.
		void write(std::shared_ptr<Reinstall::Writer> writer);

		/// @brief Add file in ISO image.
		/// @param from Source file in local filesystem.
		/// @param to File name in the image.
		void add(const char *from, const char *to);

		void set_system_area(const char *path);
		void set_volume_id(const char *volume_id);
		void set_publisher_id(const char *publisher_id);
		void set_data_preparer_id(const char *data_preparer_id);
		void set_application_id(const char *application_id);
		void set_system_id(const char *system_id);
		void set_iso_level(int level);
		void set_rockridge(int rockridge = 1);
		void set_joliet(int joliet = 1);
		void set_allow_deep_paths(int deep_paths = 1);
		void set_part_like_isohybrid();

		/// @brief Set el-torito boot options.
		void set_bootable(const char *catalog, const Settings::Boot::ElTorito &boot);

		/// @brief Set efi boot options.
		/// @param partfile	The local file with the EFI boot partition image.
		void set_bootable(const char *partfile, const Settings::Boot::Efi &boot);

	};

 }
