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
 #include <udjat/tools/quark.h>

 namespace Reinstall {

	Action::Action(const pugi::xml_node &node) : Object(node) {

		auto group = Group::find(node);

		// Load Kernel
		if(!scan(node,"kernel",[this](const pugi::xml_node &node) {
			auto rc = sources.insert(make_shared<Kernel>(node));
			if(rc.first != sources.end()) {
				info() << "Using kernel from " << (*rc.first)->url << endl;
			}
			return true;
		})) {
			throw runtime_error("Missing required entry <kernel> with the URL for installation kernel");
		}

		// Load Kernel
		if(!scan(node,"init",[this](const pugi::xml_node &node) {
			auto rc = sources.insert(make_shared<InitRD>(node));
			if(rc.first != sources.end()) {
				info() << "Using init from " << (*rc.first)->url << endl;
			}
			return true;
		})) {
			throw runtime_error("Missing required entry <init> with the URL for the linuxrc program");
		}

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
		info() << "Action " << group->c_str() << "/" << this->c_str() << " initialized with id " << group->id << "." << this->id << endl;

		// Register action.
		group->push_back(this);

	}

	Action::~Action() {
	}

	Action::Source::Source(const pugi::xml_node &node)
		: 	url(Quark(node,"url","",false).c_str()),
			path(Quark(node,"file-path","",false).c_str()),
			message(Quark(node,"download-message","",true).c_str()) {

		if(!*url) {
			throw runtime_error("Missing required attribute 'url'");
		}

	}

	void Action::pre() {
		for(auto worker : workers) {
			worker->pre();
		}
	}

	void Action::apply() {
		for(auto worker : workers) {
			worker->apply();
		}
	}

	void Action::post() {
		for(auto worker : workers) {
			worker->post();
		}
	}

	bool Action::scan(const pugi::xml_node &node, const char *tagname, const std::function<bool(const pugi::xml_node &node)> &call) {

		for(pugi::xml_node nd = node; nd; nd = nd.parent()) {
			for(pugi::xml_node child = node.child(tagname); child; child = child.next_sibling(tagname)) {
				if(call(node)) {
					return true;
				}
			}
		}
		return false;

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

