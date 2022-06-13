/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
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

 #include "private.h"
 #include <udjat/factory.h>
 #include <udjat/module.h>
 #include <udjat/moduleinfo.h>
 #include <reinstall/action.h>
 #include <reinstall/group.h>
 #include <iostream>

 using namespace std;

 Udjat::Module * udjat_module_init() {

	static const Udjat::ModuleInfo moduleinfo{"Net install builder"};

	class Module : public Udjat::Module, public Udjat::Factory {
	public:
		Module() : Udjat::Module("netinstall", moduleinfo), Udjat::Factory("option",moduleinfo) {
		}

		bool push_back(const pugi::xml_node &node) override {
			Reinstall::Group::find(node)->push_back(new NetInstall::Action(node));
			return true;
		}

	};

	return new Module();
 }
