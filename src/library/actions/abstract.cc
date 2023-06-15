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
 #include <udjat/defs.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <stdexcept>

 #include <libreinstall/action.h>
 #include <libreinstall/dialogs/progress.h>

 #include <libreinstall/writer.h>
 #include <libreinstall/writers/file.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	Action *Action::selected = nullptr;
	Action *Action::def = nullptr;

	Action::Action(const XML::Node &node) : NamedObject{node}, output{node} {

		if(getAttribute(node, "action-defaults", "default", false)) {
			Action::def = this;
		}

		// Load repositories.
		search(node,"repository",[this](const pugi::xml_node &node){
			repositories.emplace_back(node);
			return false;
		});

		// Load kernel parameters.

		// Load sources.
		search(node,"source",[this](const pugi::xml_node &node){
			sources.push_back(Source::factory(node));
			return false;
		});

		// Load templates.
		search(node,"template",[this](const pugi::xml_node &node){
			templates.emplace_back(node);
			return false;
		});

		debug("Repositories.size()=",repositories.size()," sources.size()=",sources.size()," templates.size()=",templates.size());
	}

	Action::OutPut::OutPut(const Udjat::XML::Node &node)
		: filename{XML::QuarkFactory(node,"output-file-name").c_str()},
		  length{XML::StringFactory(node,"length").as_ull()} {
	}

	Action::~Action() {
		if(Action::selected == this) {
			Action::selected = nullptr;
		}

		if(Action::def == this) {
			Action::def = nullptr;
		}

	}

	void Action::prepare(Dialog::Progress &progress, std::set<std::shared_ptr<Reinstall::Source::File>> &files) const {

		progress.run(_("Getting required files"),[this,&files](){
			for(auto source : sources) {
				source->prepare(files);
			}
		});

		if(!templates.empty()) {

			progress.run(_("Applying templates"),[this,&files](){
				for(const Reinstall::Template &tmpl : templates) {
					tmpl.apply(Udjat::Abstract::Object{},files);
				}
			});

		}

	}

	bool Action::getProperty(const char *key, std::string &value) const {


#ifdef DEBUG
		if(!Udjat::NamedObject::getProperty(key,value)) {
			Logger::Message{_("Cant get property '{}'"),key}.error(name());
			return false;
		}
		return true;
#else
		if(!Udjat::NamedObject::getProperty(key,value)) {
			throw runtime_error{Logger::Message{_("Cant get property '{}'"),key}};
		}
		return true;
#endif // DEBUG
	}


	void Action::build(Dialog::Progress &progress, std::shared_ptr<Reinstall::Builder> builder, std::set<std::shared_ptr<Reinstall::Source::File>> &files) const {

		progress.run(_("Building image"),[this,builder,&files](){
			builder->pre();
			for(auto file : files) {
				builder->push_back(file);
			}
		});

		if(!templates.empty()) {

			progress.run(_("Applying templates"),[this,builder](){
				builder->push_back(*this,templates);
			});

		}

		progress.run(_("Building image"),[this,builder](){
			builder->post();
		});

	}

	void Action::write(Dialog::Progress &progress, std::shared_ptr<Reinstall::Builder> builder, std::shared_ptr<Reinstall::Writer> writer) const {

		progress.run(_("Writing image"),[this,builder,writer](){
			builder->write(writer);
		});

	}

	std::shared_ptr<Reinstall::Builder> Action::build(Dialog::Progress &progress, std::set<std::shared_ptr<Reinstall::Source::File>> &files) const {
		auto builder = BuilderFactory();
		build(progress,builder,files);
		return builder;
	}

	void Action::write(Dialog::Progress &progress, std::shared_ptr<Reinstall::Builder> builder) const {
		write(progress,builder,WriterFactory());
	}

	std::shared_ptr<Reinstall::Builder> Action::BuilderFactory() const {
		throw runtime_error(_("The selected action is unable to build an image"));
	}

	std::shared_ptr<Reinstall::Writer> Action::WriterFactory() const {

		if(output.filename && *output.filename) {
			return make_shared<Reinstall::FileWriter>(output.filename);
		}

		throw runtime_error("Incomplete");
		/*
		return make_shared<Reinstall::USBWriter>();
		*/
	}

	void Action::activate(const ActivationType type) {
		switch(type) {
		case Selected:
			if(!Action::selected) {
				throw runtime_error(_("No selected action"));
			}
			Action::selected->activate();
			break;

		case Default:
			if(!Action::def) {
				throw runtime_error(_("No default action"));
			}
			Action::def->activate();
			break;
		}
	}

	void Action::activate(Dialog::Progress &progress) const {

		std::set<std::shared_ptr<Reinstall::Source::File>> files;

		// Step 1, get files, prepare for build.
		prepare(progress,files);

		// Step 2, build and write image.
		write(progress,build(progress,files));

	}

 }

