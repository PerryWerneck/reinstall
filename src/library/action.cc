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
 #include <reinstall/dialogs.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/protocol.h>
 #include <pugixml.hpp>

 namespace Reinstall {

	Action *Action::defaction = nullptr;

	Action & Action::getDefault() {
		if(defaction) {
			return *defaction;
		}
		throw runtime_error("No default action");
	}

	Action::Action(const pugi::xml_node &node) : Object(node) {

		scan(node, "source", [this](const pugi::xml_node &node){
			push_back(make_shared<Source>(node));
			return false;
		});

		scan(node, "kernel-parameters", [this](const pugi::xml_node &node){
			kparms.emplace_back(node);
			return false;
		});

		scan(node, "template", [this](const pugi::xml_node &node){
			push_back(make_shared<Template>(node));
			return false;
		});

		if(node.attribute("default").as_bool(false)) {
			defaction = this;
		}

		// Create action id
		static unsigned short id = 0;
		this->id = ++id;

	}

	Action::~Action() {
		if(defaction == this) {
			defaction = nullptr;
		}
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

	void Action::for_each(const std::function<void (std::shared_ptr<Source> &source)> &call) {
		for(auto source : sources) {
			call(source);
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

	bool Action::push_back(std::shared_ptr<Template> tmpl) {
		templates.push_back(tmpl);
		return true;
	}

	void Action::activate(Worker &worker) {

		{
			Dialog::Progress::getInstance().set("Initializing");
			worker.pre(*this);
		}

		// Update kernel parameters.
		{
			Dialog::Progress::getInstance().set("Getting installation parameters");
			for(KernelParameter &kparm : kparms) {
				kparm.set(*this);
			}
		}

		worker.apply(*this);
		worker.post(*this);

	}

	std::shared_ptr<Action::Source> Action::folder() {

		for(auto source : sources) {
			if( *(source->url + strlen(source->url) - 1) == '/') {
				return source;
			}
		}

		return std::shared_ptr<Source>();
	}

	void Action::load() {

		Dialog::Progress &progress = Dialog::Progress::getInstance();
		progress.set("Getting file list");

		for(std::shared_ptr<Action::Source> source = folder();source;source = folder()) {

			// Remove folder from source list.
			sources.erase(source);

			// Get index and insert folder contents.
			string index = Udjat::Protocol::WorkerFactory(string{source->url,strlen(source->url)-1}.c_str())->get();
			if(index.empty()) {
				throw runtime_error(string{"Empty response from "} + source->url);
			}

			for(auto href = index.find("<a href=\""); href != string::npos; href = index.find("<a href=\"",href)) {

				auto from = href+9;
				href = index.find("\"",from);
				if(href == string::npos) {
					throw runtime_error(string{"Unable to parse file list from "} + source->url);
				}

				string link = index.substr(from,href-from);

				if(link[0] =='/' || link[0] == '?' || link[0] == '.' || link[0] == '$')
					continue;

				if(link.size() >= 7 && strncmp(link.c_str(),"http://",7) == 0 ) {
					continue;
				}

				if(link.size() >= 8 && strncmp(link.c_str(),"https://",8) == 0 ) {
					continue;
				}

				string remote = source->url + link;
				string local = source->path + link;

				cout << "source\t" << remote << " -> " << local << endl;
				push_back(std::make_shared<Source>(remote.c_str(),local.c_str()));

				href = from+1;
			}
		}

	}

	bool Action::getProperty(const char *key, std::string &value) const noexcept {

		if(!strcasecmp(key,"kernel-parameters")) {

			for(const KernelParameter &kparm : kparms) {

				const char * val = kparm.value();

				if(val && *val) {
					if(!value.empty()) {
						value += " ";
					}

					value += kparm.name();
					value += "=";
					value += val;
				}

			}

			return true;
		}

		return Object::getProperty(key,value);
	}

	void Action::applyTemplates() {

		Dialog::Progress &progress = Dialog::Progress::getInstance();
		progress.set("Applying templates");

		for(auto tmpl : templates) {

			tmpl->load((Udjat::Object &) *this);

			for(auto source : sources) {

				if(tmpl->test(source->path)) {
					tmpl->apply(*source);
				}

			}

		}

	}

	std::shared_ptr<Action::Source> Action::find(const char *path) {

		for(auto source : sources) {

			if(source->path && *source->path && !strcmp(path,source->path)) {
				return source;
			}

		}

		throw system_error(ENOENT,system_category(),path);

	}

 }

