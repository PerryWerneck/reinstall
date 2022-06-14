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
 #include <reinstall/action.h>
 #include <reinstall/worker.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/protocol.h>

 namespace Reinstall {

	Action::Action(const pugi::xml_node &node) : Object(node) {

		/*

		TODO: Check if the file has local path, if yes, insert it as a source, if not it's just a kernel parameter.
		scan(node,"driver-installation-disk",[this](const pugi::xml_node &node) {
			Source source{node};
			auto search = sources.find(source);
			if(search == sources.end()) {
				// Insert new DUD
				sources.insert(source);
			}
			return false;
		});
		*/

		// Create action id
		static unsigned short id = 0;
		this->id = ++id;

	}

	Action::~Action() {
	}

	void Action::activate() {
		error() << "No activation code" << endl;
		throw runtime_error("Action is not available");
	}

	const char * Action::install() {
		throw runtime_error("No instalattion media");
	}

	void Action::for_each(const std::function<void (Source &source)> &call) {

		for(auto source : sources) {
			call(*source);
		}

	}

	bool Action::scan(const pugi::xml_node &node, const char *tagname, const std::function<bool(const pugi::xml_node &node)> &call) {

		for(pugi::xml_node nd = node; nd; nd = nd.parent()) {
			for(pugi::xml_node child = nd.child(tagname); child; child = child.next_sibling(tagname)) {
				if(call(child)) {
					return true;
				}
			}
		}
		return false;

	}

	bool Action::push_back(std::shared_ptr<Source> source) {
 		return (sources.insert(source).first != sources.end());
	}

	void Action::activate(Worker &worker) {

		worker.pre(*this);
		worker.apply(*this);
		worker.post(*this);

	}

	void Action::scanForSources(const pugi::xml_node &node, const char *tagname) {

		/*
		scan(node,tagname,[this](const pugi::xml_node &node) {
			Source source{node};
			auto search = sources.find(source);
			if(search == sources.end()) {
				sources.insert(source);
			}
			return false;
		});
		*/

	}

 }

