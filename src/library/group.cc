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

 #include <config.h>
 #include <reinstall/controller.h>
 #include <udjat/tools/logger.h>
 #include <memory>

 using namespace std;

 namespace Reinstall {

	std::shared_ptr<Group> Group::factory(const pugi::xml_node &node) {

		std::shared_ptr<Group> rc;
		const char *name = node.attribute("name").as_string("default");
		Controller &controller = Controller::getInstance();

		controller.for_each([&rc,name](std::shared_ptr<Group> group){

			if(*group == name) {
				rc = group;
				return true;
			}
			return false;

		});

		if(!rc) {
			rc = make_shared<Group>(node);
			controller.push_back(rc);
		}

		rc->setup(node);

		return rc;
	}

	std::shared_ptr<Group> Group::find(const pugi::xml_node &node) {

		std::shared_ptr<Group> rc;
		const char *name = node.attribute("group-name").as_string("");
		Controller &controller = controller.getInstance();

		while(!*name) {

			auto parent = node.parent();
			if(!parent) {
				throw runtime_error("Can't determine group, use the 'group-name' attribute to set it");
			}

			if(strcasecmp(parent.name(),"group") == 0) {
				name = parent.attribute("name").as_string("");
			}

		}

		controller.for_each([&rc,name](std::shared_ptr<Group> group){

			if(*group == name) {
				rc = group;
				return true;
			}
			return false;

		});

		if(!rc) {
			throw runtime_error(string{"Cant find group '"} + name + "'");
		}

		return rc;

	}

	Group::Group(const pugi::xml_node &node) : Reinstall::Object(node) {

		static unsigned short id = 0;
		this->id = ++id;

		if(title) {
			title.get_style_context()->add_class("group-title");
		}

		if(subtitle) {
			subtitle.get_style_context()->add_class("group-subtitle");
		}

		Udjat::Logger::String{"Group '",name(),"' initialized with id ",this->id}.trace("group");

	}

	Group::~Group() {
	}

	bool Controller::for_each(const std::function<bool (std::shared_ptr<Group> group)> &call) const {

		for(auto group : groups) {
			if(call(group)) {
				return true;
			}
		}
		return false;

	}

	void Group::push_back(std::shared_ptr<Action> action) {
		actions.push_back(action);
		Udjat::Logger::String{"Action '",name(),"/",action->name(),"' initialized with id ",id,".",action->id}.trace("group");
	}

	bool Group::for_each(const std::function<bool (std::shared_ptr<Action> action)> &call) const {

		for(auto action : actions) {
			if(call(action)) {
				return true;
			}
		}

		return false;

	}

 }
