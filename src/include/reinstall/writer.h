/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2022 Perry Werneck <perry.werneck@gmail.com>
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
 #include <cstddef>
 #include <memory>

 namespace Reinstall {

	/// @brief Image Writer
	class UDJAT_API Writer {
	private:

	protected:
		Writer();
		virtual ~Writer();

	public:

		/// @brief Open Device for writing
		virtual void open() = 0;

		/// @brief Write data do device.
		virtual void write(const void *buf, size_t count) = 0;

		virtual void finalize() = 0;

		/// @brief Close Device.
		virtual void close() = 0;

		/// @brief Factory file writer.
		static std::shared_ptr<Writer> FileFactory(const char *filename);

		/// @brief Detect USB storage device, create writer for it.
		/// @param length Required device size (0 to ignore it);
		static std::shared_ptr<Writer> USBStorageFactory(size_t length = 0);

	};

 }
