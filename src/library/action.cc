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

		scan(node, "kernel-parameter", [this](const pugi::xml_node &node){
			kparms.emplace_back(node);
			return false;
		});

		scan(node, "repository", [this](const pugi::xml_node &node){
			repositories.insert(make_shared<Repository>(node));
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

	void Action::load() {

		Dialog::Progress &progress = Dialog::Progress::getInstance();

		progress.set("Updating file sources");
		for(auto source : sources) {
			source->set(*this);
		}

		progress.set("Getting file list");
		{
			std::vector<std::shared_ptr<Source>> contents;

			// Expand all sources.
			for(auto source = sources.begin(); source != sources.end();) {
				if((*source)->contents(*this,contents)) {
					source = sources.erase(source);
				} else {
					source++;
				}
			}

			// Add expanded elements.
			for(auto source : contents) {
				sources.insert(source);
			}

		}

	}

	bool Action::getProperty(const char *key, std::string &value) const noexcept {

		if(!strcasecmp(key,"kernel-parameters")) {

			if(kparms.empty()) {
				warning() << "The kernel parameters list is empty" << endl;
				value.clear();
			} else {

				for(const KernelParameter &kparm : kparms) {

					const char * name = kparm.name();
					if(!(name && *name)) {
						error() << "Unnamed kernel parameter, possible misconfiguration" << endl;
						continue;
					}

					const char * val = kparm.value();
					if(val && *val) {
						if(!value.empty()) {
							value += " ";
						}

						value += name;
						value += "=";
						value += val;
					}

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

	std::shared_ptr<Source> Action::source(const char *path) {
		for(auto source : sources) {
			if(source->path && *source->path && !strcmp(path,source->path)) {
				return source;
			}
		}
		error() << "Cant find source for path '" << path << "'" << endl;
		throw system_error(ENOENT,system_category(),path);
	}

	std::shared_ptr<Repository> Action::repository(const char *name) const {

		if(repositories.empty()) {
			throw runtime_error("No repositories on this action");
		}

		cout << "searching for repository '" << name << "' in " << repositories.size() << " repos" << endl;

		for(auto repository : repositories) {
			if(*repository == name) {
				return repository;
			}
		}

		throw system_error(ENOENT,system_category(),name);
	}

 }

