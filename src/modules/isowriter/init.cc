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

 #include <config.h>

 /*
 #include <udjat/module.h>
 #include <udjat/factory.h>

 #include <libreinstall/action.h>

 using namespace std;
 using namespace Udjat;
 using namespace Reinstall;

 Udjat::Module * udjat_module_init() {

 	static const Udjat::ModuleInfo moduleinfo { "USB/ISO Writer module" };

	class Module : public Udjat::Module, public Udjat::Factory {
	public:
		Module() : Udjat::Module("ImageWriter", moduleinfo), Udjat::Factory("iso-writer",moduleinfo) {
		}

		bool generic(const pugi::xml_node &node) override {

			if(node.attribute("url")) {
				/// @brief ISO Writer Action
				Reinstall::push_back(node,make_shared<IsoWriter>(node));
			}

			Reinstall::push_back(node,make_shared<DiskWriter>(node));

			return true;

		}

	};

	return new Module();

 }
 */

