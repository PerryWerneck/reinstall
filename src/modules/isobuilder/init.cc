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
 #include <udjat/module.h>
 #include <udjat/factory.h>
 #include <udjat/tools/string.h>

 #include <reinstall/actions/isobuilder.h>
 #include <reinstall/actions/fsbuilder.h>

 using namespace std;
 using namespace Udjat;

 Udjat::Module * udjat_module_init() {

 	static const Udjat::ModuleInfo moduleinfo { "Disk image builder" };

	class Module : public Udjat::Module, public Udjat::Factory {
	public:
		Module() : Udjat::Module("iso-builder", moduleinfo), Udjat::Factory("iso-builder",moduleinfo) {
		}

		bool generic(const pugi::xml_node &node) override {

			switch(String{node,"filesystem","iso-9660"}.select("iso-9660","fat32",nullptr)) {
			case 0:	// iso9660
				Reinstall::push_back(node,make_shared<Reinstall::IsoBuilder>(node));
				break;

			case 1: // fat32
				Reinstall::push_back(node,make_shared<Reinstall::FSBuilder>(node));
				break;

			default:
				throw runtime_error("The attribute 'filesystem' is invalid");

			}


			return true;
		}

	};

	return new Module();

 }

