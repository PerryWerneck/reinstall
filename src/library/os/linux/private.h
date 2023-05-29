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

 #include <config.h>
 #include <udjat/defs.h>
 #include <reinstall/writer.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	namespace Device {

		/// @brief Loop device.
		class UDJAT_PRIVATE Loop : public std::string {
		private:

			long devnumber; ///< @brief Loop device number.

			struct {
				int control = -1;
				int image = -1;
				int device = -1;
			} fd;

		public:
			Loop();
			Loop(const char *filename);
			~Loop();

			/// @brief Bind filename in loop device.
			void bind(const char *filename);

			/// @brief Disconnect device from file.
			void unbind();

		};


	}

 }
