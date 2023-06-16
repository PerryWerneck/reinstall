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
 #include <unordered_map>

 #include <libreinstall/action.h>
 #include <libreinstall/repository.h>
 #include <libreinstall/dialogs/progress.h>

 #include <libreinstall/writer.h>
 #include <libreinstall/writers/file.h>

 #include <libreinstall/kernelparameter.h>

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
		search(node,"kernel-parameter",[this](const pugi::xml_node &node){

			auto reponame = Udjat::XML::StringFactory(node,"repository");

			if(reponame.empty()) {

				// It's a normal parameter
				kparms.push_back(Kernel::Parameter::factory(node));

			} else {

				// It's a repository parameter.
				const_cast<Repository &>(repository(reponame.c_str())).set_kernel_parameter(node);

			}

			return false;
		});

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

	const Reinstall::Repository & Action::repository(const char *name) const {

		for(const Repository &repository : repositories) {
			if(repository == name) {
				return repository;
			}
		}

		throw runtime_error(Logger::String("Cant find repository '",name,"'"));

	}

	void Action::prepare(Dialog::Progress &progress, std::set<std::shared_ptr<Reinstall::Source::File>> &files) const {

		progress.run(_("Getting required files"),[this,&progress,&files](){

			/// @brief Resolved repositories.
			std::unordered_map<std::string, Udjat::URL> urls;

			for(auto source : sources) {

				URL remote{source->remote()};

				const char *reponame = source->repository();
				if(reponame && *reponame && (remote[0] == '/' || remote[0] == '.')) {

					// Using repository, resolve real URLs
					const Repository &repo = this->repository(reponame);

					auto it = urls.find(string{reponame});
					if(it == urls.end()) {

						// Not in cache, search for repository real URL
						auto url = repo.url();

						auto result = urls.insert({string{reponame},url});
						if(!result.second) {
							throw runtime_error(Logger::String{"Unable to insert repository '",reponame,"' on URL cache"});
						}

						it = result.first;

					}

					// Resolve local path.
					URL local{source->local()};
					if(local.empty()) {
						local = repo.path(local.ComponentsFactory().path.c_str());
					}

					// Get file list
					source->prepare(
						local,									// URL on local file system.
						(it->second + remote.c_str()),			// URL for remote repository.
						files									// File list.
					);

				} else {

					// Not using repository, just get file list.
					source->prepare(files);

				}
				/*
				URL local;

				if(reponame && *reponame && (remote[0] == '/' || remote[0] == '.')) {

					// source is relative to repository, get real URL.
					auto it = urls.find(string{reponame});
					if(it == urls.end()) {

						// Not resolved, get URL.
						debug("Searching for repository '",reponame,"'");

						for(const Reinstall::Repository &repository : repositories) {

							if(repository == reponame) {

								// Resolve remote path.
								auto url = repository.url();

								auto result = urls.insert({string{reponame},url});
								if(!result.second) {
									throw runtime_error(Logger::String{"Unable to insert repository '",reponame,"' on URL cache"});
								}

								it = result.first;

								break;
							}
						}

						if(it == urls.end()) {
							throw runtime_error(Logger::String{"Cant find repository '",reponame,"'"});
						}

					}

					// Gt the real URL, prepare it.
					progress.set_sub_title(_("Getting required files"));

					source->prepare(
						local,								// Local path
						(it->second + remote.c_str()),		// Remote path
						files								// File list
					);

				} else {

					// Source is not relative, just prepare it.
					source->prepare(files);

				}
				*/

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

		if(!strcasecmp(key,"kernel-parameters")) {

			for(auto kparm : kparms) {

				if(!value.empty()) {
					value += " ";
				}

				value += kparm->name();

				auto val = kparm->value();
				if(!val.empty()) {
					value += "=";
					value += kparm->value();
				}

			}

			for(const Repository &repository : repositories) {

				const Kernel::Parameter &kparm{repository.kernel_parameter()};

				if(!kparm) {
					continue;
				}

				if(!value.empty()) {
					value += " ";
				}

				value += kparm.name();

				auto val = kparm.value();
				if(!val.empty()) {
					value += "=";
					value += kparm.value();
				}

			}

			Logger::String{"Kernel-parameters: '",value,"'"}.write(Logger::Debug,name());

			return true;
		}

		if(!Udjat::NamedObject::getProperty(key,value)) {
			throw runtime_error{Logger::Message{_("Cant get property '{}'"),key}};
		}
		return true;

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

	void Action::activate(Dialog::Progress &progress, const ActivationType type) {
		switch(type) {
		case Selected:
			if(!Action::selected) {
				throw runtime_error(_("No selected action"));
			}
			Action::selected->activate(progress);
			break;

		case Default:
			if(!Action::def) {
				throw runtime_error(_("No default action"));
			}
			Action::def->activate(progress);
			break;
		}
	}

	void Action::activate(Dialog::Progress &progress) const {

		std::set<std::shared_ptr<Reinstall::Source::File>> files;

		// Step 1, get files, prepare for build.
		Logger::String{"Preparing"}.info(name());
		prepare(progress,files);

		// Step 2, build and write image.
		write(progress,build(progress,files));

	}

 }

