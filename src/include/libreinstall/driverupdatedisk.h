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
  * @brief Declare the driver update disk class.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <memory>
 #include <libreinstall/source.h>
 #include <libreinstall/kernelparameter.h>

 namespace Reinstall {

	class UDJAT_API DriverUpdateDisk : public Source, public Kernel::Parameter {
	public:
		DriverUpdateDisk(const Udjat::XML::Node &node);

		virtual ~DriverUpdateDisk();

		/// @brief Get kernel parameter value.
		/// @see Kernel::Parameter
		const std::string value() const override;

		static std::shared_ptr<DriverUpdateDisk> factory(const Udjat::XML::Node &node);

		void prepare(const Udjat::URL &local, const Udjat::URL &remote, Files &files) const override;

	};

 }
