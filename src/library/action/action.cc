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
 #include <reinstall/builder.h>
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
		: 	Reinstall::Abstract::Object(node), options{node},
			output_file{getAttribute(node,"output-file","")},
			icon_name{getAttribute(node,"icon",iname)},
			item{UserInterface::getInstance().ActionFactory(node,icon_name)} {

		if(node.attribute("default").as_bool(false) || !selected) {
			selected = this;
		}

		// Get dialogs
		scan(node, "dialog", [this](const pugi::xml_node &node){

			String name{node,"name","unnamed",false};

			debug("Dialog: ",name);

			switch(name.select("confirmation","success","failed",nullptr)) {
			case 0:	// Confirmation
				info() << "Using customized confirmation dialog" << endl;
				dialog.confirmation.set(node);
				break;

			case 1:	// success
				info() << "Using customized success dialog" << endl;
				dialog.success.set(node);
				break;

			case 2:	// failed
				info() << "Using customized error dialog" << endl;
				dialog.failed.set(node);
				break;

			default:
				warning() << "Unknown dialog '" << name << "'" << endl;

			}

			return false;
		});

		scan(node, "source", [this](const pugi::xml_node &node){
			push_back(make_shared<Source>(node));
			return false;
		});

		scan(node, "kernel-parameter", [this](const pugi::xml_node &node){
			kparms.emplace_back(node);
			return false;
		});

		// Get driver updates
		scan(node,"driver-update-disk",[this](const pugi::xml_node &node) {

			if(node.attribute("path")) {

				// Has path, setup as a source.
				auto source = make_shared<Reinstall::Source>(node);
				push_back(source);

				// Setup DUD url based on local path.
				kparms.emplace_back("dud",source->hd_path());

			} else if(node.attribute("url")) {

				// No path, setup only as kernel parameter 'dud=url'.
				kparms.emplace_back("dud",node);

			} else {

				throw runtime_error(_("DUD Definition requires path or url attribute."));

			}

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

		// Get scripts.
		scan(node,"script",[this](const pugi::xml_node &node) {
			switch(Udjat::String{node.attribute("type").as_string("post")}.select("pre","post",nullptr)) {
			case 0:	// pre.
				scripts.pre.emplace_back(node);
				break;

			case 1: // post.
				scripts.post.emplace_back(node);
				break;

			default:
				throw runtime_error(_("Invalid 'type' attribute"));

			}
			return true;
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

	std::shared_ptr<Reinstall::Builder> Action::BuilderFactory() {
		throw runtime_error(_("No available image builder"));
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

	void Action::load() {

		Dialog::Progress &progress = Dialog::Progress::getInstance();

		info() << "Getting " << sources.size() << " required file list" << endl;

		progress.set_sub_title(_("Getting required files"));

		// Expand all sources.
		for(auto iter = sources.begin(); iter != sources.end();) {

			std::shared_ptr<Source> source = *iter;
			std::vector<std::shared_ptr<Source>> contents;

			iter = sources.erase(iter);	// Remove it; path can change.

			source->set(*this);

			if(!source->contents(*this,contents)) {
				contents.push_back(source);
			}

			Logger::String {
				"Source '", source->name(), "' has ", contents.size(), " file(s)"
			}.trace(name());

			// Add expanded elements.
			for(std::shared_ptr<Source> source : contents) {
				if(sources.count(source)) {
					Logger::String{"Duplicate file '",source->path,"' on source ",source->name()}.trace(name());
				} else {
					sources.insert(source);
				}
			}

		}

		info() << "Got " << sources.size() << " files to get" << endl;

	}

	std::shared_ptr<Source> Action::source(const char *path) const {
		for(auto source : sources) {
			debug(source->path);
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

		// cout << "searching for repository '" << name << "' in " << repositories.size() << " repos" << endl;

		for(auto repository : repositories) {
			if(*repository == name) {
				return repository;
			}
		}

		throw system_error(ENOENT,system_category(),name);
	}

	bool Action::getProperty(const char *key, std::string &value) const {

		debug("Searching for '",key,"' in ",name());

		if(strcasecmp(key,"icon-name") == 0) {

			value = get_icon_name();
			return true;

		}

		if(strcasecmp(key,"label") == 0 || strcasecmp(key,"install-label") == 0 ) {

			value = get_label();
			return true;

		}

		if(strcasecmp(key,"kernel-parameters") == 0) {

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

					std::string val = kparm.expand(*this);
					if(val.empty()) {
						warning() << "Kernel parameter '" << name << "' is empty, ignoring" << endl;
					} else {

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

		if(strcasecmp(key,"boot-dir") == 0) {
			value = "/boot/x86_64";
			return true;
		}

		if(strcasecmp(key,"kernel-file") == 0) {
			value = "/boot/x86_64/loader/linux";
			return true;
		}

		if(strcasecmp(key,"initrd-file") == 0) {
			value = "/boot/x86_64/loader/initrd";
			return true;
		}

		return Reinstall::Abstract::Object::getProperty(key,value);

	}

	std::shared_ptr<Reinstall::Builder> Action::pre() {

		Dialog::Progress &dialog = Dialog::Progress::getInstance();

		dialog.set_sub_title(_("Preparing"));
		for(Script &script : scripts.pre) {
			script.run(*this);
		}

		dialog.set_sub_title(_("Initializing"));
		auto builder = BuilderFactory();
		builder->pre(*this);

		// Get folder contents.
		dialog.set_sub_title(_("Getting file lists"));
		load();

		// Apply templates.
		dialog.set_sub_title(_("Checking for templates"));
		for(auto tmpl : templates) {

			// Load template
			tmpl->load((Udjat::Object &) *this);

			// Apply it on sources (if necessary).
			for(auto source : sources) {

				if(tmpl->test(source->path)) {
					tmpl->apply(*source);
				}

			}

			{
				const char *path = tmpl->get_path();
				if(path && *path) {

					// Template has filename, update or create source
					Udjat::String str{path};
					str.expand(*this);

					debug("Template=",tmpl->c_str()," filename=",tmpl->get_filename());
					auto source = make_shared<Source>(
							tmpl->c_str(),
							(string{"file://"} + tmpl->get_filename()).c_str(),
							str.c_str()
					);

					auto it = sources.find(source);
					if(it != sources.end()) {
						sources.erase(it);
					}

					sources.insert(source);

				}
			}

		}

		// Download files.
		dialog.set_sub_title(_("Getting required files"));
		size_t total = source_count();
		size_t current = 0;

		info() << "Getting " << total << " required files" << endl;
		for_each([this,&current,total,&dialog,builder](Source &source) {
			dialog.set_count(++current,total);
			Logger::String(source.url," (",current,"/",total,")").trace(source.name());
			builder->apply(source);
		});
		dialog.set_count(0,0);

		info() << "Calling 'build' methods" << endl;
		dialog.set_sub_title(_("Building"));
		builder->build(*this);

		info() << "Calling 'post' methods" << endl;
		dialog.set_sub_title(_("Building"));
		builder->post(*this);

		return builder;
	}

	void Action::post(std::shared_ptr<Reinstall::Writer> UDJAT_UNUSED(writer)) {
		Dialog::Progress &dialog = Dialog::Progress::getInstance();

		dialog.set_sub_title(_("Running post scripts"));
		for(Script &script : scripts.post) {
			script.run(*this);
		}
	}

	void Action::activate() {
		post(pre()->burn(WriterFactory()));
	}

 }

