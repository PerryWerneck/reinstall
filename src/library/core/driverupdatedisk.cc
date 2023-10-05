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
  * @brief Implements driver update disk.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/configuration.h>
 #include <libreinstall/source.h>
 #include <libreinstall/driverupdatedisk.h>
 #include <libreinstall/source.h>
 #include <libreinstall/kernelparameter.h>
 #include <string>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	DriverUpdateDisk::DriverUpdateDisk(const Udjat::XML::Node &node) : Source{node}, Kernel::Parameter{"dud"} {
	}

	DriverUpdateDisk::~DriverUpdateDisk() {
	}

	void DriverUpdateDisk::prepare(const Udjat::URL &local, const Udjat::URL &remote, Files &files) const {
		if(this->imgpath && *this->imgpath) {
			Source::prepare(local,remote,files);
		}
	}

	const std::string DriverUpdateDisk::value() const {
		if(this->imgpath && *this->imgpath) {
			return Config::Value<string>{"schemes","disk","disk:"} + this->imgpath;
		}
		return remote();
	}

	std::shared_ptr<DriverUpdateDisk> DriverUpdateDisk::factory(const Udjat::XML::Node &node) {
		return make_shared<DriverUpdateDisk>(node);
	}

 }
