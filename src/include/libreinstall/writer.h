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
 #include <udjat/ui/dialogs/progress.h>
 #include <udjat/tools/file/handler.h>
 #include <memory>

 namespace Reinstall {

	/// @brief The default image writer.
	class UDJAT_API Writer {
	private:

		///< @brief Device set by user, disable detection dialog.
		static const char *devicename;

		///< @brief Device length set by user, used only if devicename is set.
		static unsigned long long devicelength;

	protected:
		Writer() = default;

	public:

		/// @brief Write data to device at offset.
		/// @param contents Data to write.
		/// @param length Data length.
		/// @return Number of bytes written (allways 'length')
		virtual size_t write(unsigned long long offset, const void *contents, size_t length) = 0;

		/// @brief Write file to device.
		/// @param file The file to write.
		void write(Udjat::Dialog::Progress &dialog, Udjat::File::Handler &file);

		/// @brief Set a fixed device name.
		/// @param devicename The device name.
		static void set_device_name(const char *devicename);

		/// @brief Set length for fixed device name.
		/// @param length Bytes to allocate on device (if it's a file).
		static void set_device_length(unsigned long long length);

		/// @brief Detect USB device, build an image writer for it.
		static std::shared_ptr<Writer> factory(const char *title);

		/// @brief Get device length.
		/// @return Device length (0 if cant be determined).
		virtual unsigned long long size();

		/// @brief Finalize writing
		virtual void finalize();

	};

 }

