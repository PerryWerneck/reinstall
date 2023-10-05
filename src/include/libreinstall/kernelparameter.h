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
  * @brief Brief description of this source.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <memory>

 namespace Reinstall {

	namespace Kernel {

		class UDJAT_API Parameter {
		protected:
			const char *nm = nullptr;

		public:

			static std::shared_ptr<Parameter> factory(const Udjat::XML::Node &node);

			constexpr Parameter(const char *name) : nm{name} {
			}

			inline operator bool() const {
				return (nm && *nm);
			}

			Parameter(const Udjat::XML::Node &node);

			inline const char *name() const noexcept {
				return nm;
			}

			virtual const std::string value() const = 0;

		};


	}

 }

