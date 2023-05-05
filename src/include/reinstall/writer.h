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
 #include <reinstall/action.h>
 #include <string>

 namespace Reinstall {

	/// @brief Image Writer
	class UDJAT_API Writer {
	private:

		static const char * usbdevname;

	protected:

		typedef Reinstall::Writer super;

#ifndef _WIN32

		void write(int fd, const void *buf, size_t count);
		void finalize(int fd);

#endif // _WIN32

	public:

		Writer(const Reinstall::Action &action);
		virtual ~Writer();

		/// @brief Set USB device name.
		static void setUsbDeviceName(const char *name);

		/// @brief Open Device for writing
		virtual void open();

		/// @brief Write data do device.
		virtual void write(const void *buf, size_t count);

		virtual void finalize();

		/// @brief Close Device.
		virtual void close();

		/// @brief Factory file writer.
		static std::shared_ptr<Writer> FileWriterFactory(const Reinstall::Action &action, const char *filename);

		/// @brief Detect USB storage device, create writer for it.
		/// @param length Required device size (0 to ignore it);
		static std::shared_ptr<Writer> USBWriterFactory(const Reinstall::Action &action, size_t length = 0);

	};

	class UDJAT_API FileWriter : public Writer {
	private:
		int fd = -1;
		std::string filename;

	public:
		FileWriter(const Reinstall::Action &action, const char *filename);
		virtual ~FileWriter();

		void open() override;
		void close() override;
		void finalize() override;

		void write(const void *buf, size_t length);

	};

 }

