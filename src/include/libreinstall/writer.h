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
 #include <memory>

 namespace Reinstall {

	/// @brief The default image writer.
	class UDJAT_API Writer {
	private:
		static const char *devicename;	///< @brief Device set by user, disable detection dialog.
		int fd = -1; ///< @brief Device handler.

	protected:
		Writer() = default;

	public:
		~Writer();

		/// @brief Set a fixed device name.
		/// @param devicename The device name.
		/// @param length Bytes to allocate on device (if it's a file).
		static set_device_name(const char *devicename, unsigned long long length = 0LL);

		/// @brief Detect USB device, build an image writer for it.
		static std::shared_ptr<Writer> factory();

		/// @brief Set image size, notify user if not enough space.
		/// @return true to continue, false to stop.
		virtual bool prepare(unsigned long long image_len = 0);

	};

 }

