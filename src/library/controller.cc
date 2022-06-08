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
 #include <udjat/moduleinfo.h>
 #include <reinstall/controller.h>

 namespace Reinstall {

	static const Udjat::ModuleInfo moduleinfo{PACKAGE_NAME " top menu"};

	Controller::Controller() : Udjat::Factory("group",moduleinfo) {
	}

	Controller & Controller::getInstance() {
		static Controller controller;
		return controller;
	}

	std::shared_ptr<Group> Controller::find(const pugi::xml_node &node) {

		const char * name = node.attribute("name").as_string("default");

		for(auto group : groups) {
			if(*group == name) {
				return group;
			}
		}

		auto group = make_shared<Group>(node);
		groups.push_back(group);
		return group;

	}

	bool Controller::push_back(const pugi::xml_node &node) {
		find(node);
		return true;
	}

 }

