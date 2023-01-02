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
 #include <reinstall/action.h>
 #include <reinstall/worker.h>
 #include <reinstall/dialogs.h>
 #include <reinstall/userinterface.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/logger.h>
 #include <pugixml.hpp>
 #include <udjat/tools/intl.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	UDJAT_API void push_back(const pugi::xml_node &node, std::shared_ptr<Action> action) {
		Reinstall::Abstract::Group::find(node)->push_back(action);
	}

	Action *Action::selected = nullptr;

	Action & Action::get_selected() {
		if(selected) {
			return *selected;
		}
		throw runtime_error(_("No action"));
	}

	bool Action::Options::Factory(const pugi::xml_node &node, const char *attrname, bool def) {

		auto attribute = node.attribute(attrname);

		if(!attribute) {
			return def;
		}

		const char * str = attribute.as_string();
		if(strstr(str,"://")) {

			// It's an URL, test it.
			try {

				return URL(str).test() == 200;

			} catch(const exception &e) {

				cerr << "option\t" << str << ": " << e.what() << endl;

			}

			return false;

		}

		return attribute.as_bool(def);
	}

	Action::Options::Options(const pugi::xml_node &node)
		: enabled(Factory(node,"enabled",true)),
		  visible(Factory(node,"visible",true)),
		  reboot(Factory(node,"visible",false)),
		  quit(Factory(node,"visible",true))
		 {
	}

	Action::Action(const pugi::xml_node &node, const char *iname)
		: 	Udjat::NamedObject(node), options{node},
			output_file{getAttribute(node,"output-file","")},
			icon_name{getAttribute(node,"icon",iname)},
			item{UserInterface::getInstance().ActionFactory(node,icon_name)} {

		if(node.attribute("default").as_bool(false) || !selected) {
			selected = this;
		}

		// Get dialogs
		for(pugi::xml_node parent = node; parent; parent = parent.parent()) {

			for(auto child = parent.child("dialog"); child; child = child.next_sibling("dialog")) {

				switch(String{child,"name"}.select("confirmation","success","failed",nullptr)) {
				case 0: // confirmation.
					if(!dialog.confirmation) {
						dialog.confirmation.set(child);
					}
					break;
				case 1: // success
					if(!dialog.success) {
						dialog.success.set(child);
					}
					break;
				case 2: // failed
					if(!dialog.failed) {
						dialog.failed.set(child);
					}
					break;
				default:
					warning() << "Unexpected dialog name '" << String{child,"name"} << "'" << endl;
				}

			}

		}

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

		// Create action id
		static unsigned short id = 0;
		this->id = ++id;

	}

	Action::~Action() {
		if(selected == this) {
			selected = nullptr;
		}
	}

	std::shared_ptr<Reinstall::Worker> Action::WorkerFactory() {
		return make_shared<Reinstall::Worker>();
	}

	std::shared_ptr<Reinstall::Writer> Action::WriterFactory() {
		if(output_file && *output_file) {
			return Reinstall::Writer::FileWriterFactory(*this,output_file);
		}
		return Reinstall::Writer::USBWriterFactory(*this);
	}

	bool Action::interact() {
		return true;
	}

	const char * Action::install() {
		throw runtime_error(_("No installation media"));
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

	void Action::prepare(Worker &worker) {

		Dialog::Progress &dialog = Dialog::Progress::getInstance();

		{
			dialog.set_title(_("Initializing"));
			worker.pre(*this);

			// Get folder contents.
			dialog.set_title(_("Getting file lists"));
			load();

			// Apply templates.
			dialog.set_title(_("Checking for templates"));
			applyTemplates();

			// Download files.
			dialog.set_title(_("Getting required files"));
			size_t total = source_count();
			size_t current = 0;
			for_each([this,&current,total,&dialog,&worker](Source &source) {
				dialog.set_count(++current,total);
				worker.apply(source);
			});
			dialog.set_count(0,0);

		}

		// Update kernel parameters.
		{
			dialog.set_title(_("Getting installation parameters"));
			for(KernelParameter &kparm : kparms) {
				kparm.set(*this);
			}
		}

		worker.post(*this);

	}

	void Action::load() {

		Dialog::Progress &progress = Dialog::Progress::getInstance();
		progress.set_title(_("Getting required files"));

		for(auto source : sources) {
			source->set(*this);
		}

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

	/*
	bool Action::getProperty(const char *key, std::string &value) const noexcept {

		if(!strcasecmp(key,"kernel-parameters")) {

			if(kparms.empty()) {
				warning() << _("The kernel parameters list is empty") << endl;
				value.clear();
			} else {

				for(const KernelParameter &kparm : kparms) {

					const char * name = kparm.name();
					if(!(name && *name)) {
						error() << _("Unnamed kernel parameter, possible misconfiguration") << endl;
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
	*/

	void Action::applyTemplates() {

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
			throw runtime_error(_("No repositories on this action"));
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

