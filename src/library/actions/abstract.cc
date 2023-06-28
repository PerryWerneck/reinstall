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
 #include <udjat/tools/singleton.h>
 #include <stdexcept>
 #include <unordered_map>
 #include <vector>

 #include <libreinstall/action.h>
 #include <libreinstall/repository.h>
 #include <udjat/ui/dialogs/progress.h>

 #include <libreinstall/writer.h>
 #include <libreinstall/writers/file.h>
 #include <libreinstall/writers/usb.h>

 #include <libreinstall/kernelparameter.h>

 #include <unistd.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	Action *Action::selected = nullptr;
	Action *Action::def = nullptr;

	Action::Action(const XML::Node &node) : NamedObject{node}, Udjat::Menu::Item{node}, dialogs{node}, output{node} {

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

	void Action::prepare(Dialog::Progress &progress, Source::Files &files) const {

		progress.message(_("Getting required files"));
		{

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
					URL local{source->filename()};

					if(local.empty()) {
						local = repo.path(remote.ComponentsFactory().path.c_str());
					} else {
						local = repo.path(local.ComponentsFactory().path.c_str());
					}

					debug("Local=",local.c_str());

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
			}
		}

		if(!templates.empty()) {

			progress.message(_("Applying templates"));
			for(const Reinstall::Template &tmpl : templates) {
				files.apply(*this,tmpl);
			}

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

	void Action::build(Dialog::Progress &progress, std::shared_ptr<Reinstall::Builder> builder, Source::Files &files) const {

		progress.pulse();
		progress.message(_("Building image"));
		{
			builder->pre();
			files.for_each([builder](std::shared_ptr<Source::File> file){
				builder->push_back(file);
			});

		}

		if(!templates.empty()) {
			progress.message(_("Applying templates"));
			builder->push_back(*this,templates);
		}

		progress.message(_("Building image"));
		builder->post();

		progress.message(_("Build process complete"));
	}

	void Action::write(Dialog::Progress &progress, std::shared_ptr<Reinstall::Builder> builder, std::shared_ptr<Reinstall::Writer> writer) const {

		progress.pulse();
		progress.message(_("Writing image"));
		builder->write(writer);

	}

	std::shared_ptr<Reinstall::Builder> Action::BuilderFactory() const {
		throw runtime_error(_("The selected action is unable to build an image"));
	}

	std::shared_ptr<Reinstall::Writer> Action::WriterFactory() const {

		if(output.filename && *output.filename) {
			return make_shared<Reinstall::FileWriter>(output.filename);
		}

		return Reinstall::UsbWriter::factory(dialogs.title);

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

	void Action::activate() {

		class Files : public Reinstall::Source::Files {
		private:
			std::set<std::shared_ptr<Reinstall::Source::File>> files;

		public:
			void insert(std::shared_ptr<Reinstall::Source::File> file) override {
				files.insert(file);
			}

			void for_each(const std::function<void(std::shared_ptr<Reinstall::Source::File>)> &worker) override {
				for(auto file : files) {
					worker(file);
				}
			}

			void remove_if(const std::function<bool(const Reinstall::Source::File &)> &worker) override {

				for(auto it = files.begin(); it != files.end();) {
					if(worker(*(*it))) {
						it = files.erase(it);
					} else {
						it++;
					}
				}

			}

			void apply(const Udjat::Abstract::Object &object,const Template &tmpl) override {

				vector<string> updates;

				for(auto it = files.begin(); it != files.end();) {

					std::shared_ptr<Reinstall::Source::File> file = *it;

					if(tmpl.test(file->c_str())) {

						Logger::String{"Replacing '",file->c_str(),"'"}.trace(tmpl.name());
						it = files.erase(it);
						updates.push_back(file->c_str());

					} else {

						it++;

					}

				}

				if(updates.size()) {

					// Have updates, get template.
					String text = tmpl.get();
					text.expand(object);

					if(Logger::enabled(Logger::Debug)) {
						Logger::String{"New contents:\n",text.c_str(),"\n"}.write(Logger::Debug,tmpl.name());
					}

					for(string &path : updates) {
						insert(Source::File::Factory(path.c_str(),text.c_str()));
					}

				}

			}

		};

		if(!dialogs.confirmation.confirm()) {
			Logger::String{"Cancel by user choice"}.info(name());
			return;
		}

		Dialog::Controller &dcntrl{Dialog::Controller::instance()};

		try {

			Files files;

			// Step 1, get files, prepare for build.
			Logger::String{"Preparing"}.info(name());
			dcntrl.run(dialogs.progress,[this,&files](Dialog::Progress &dialog) {
				dialog.title(dialogs.title);
				dialog.message(_("Preparing"));
				prepare(dialog,files);
				return 0;
			});

			// Step 2, build image.
			Logger::String{"Building"}.info(name());
			std::shared_ptr<Reinstall::Builder> builder{BuilderFactory()};
			dcntrl.run(dialogs.progress,[this,builder,&files](Dialog::Progress &dialog) {
				dialog.title(dialogs.title);
				dialog.message(_("Building"));
				build(dialog,builder,files);
				return 0;
			});

			// Step 3, get writer.
			std::shared_ptr<Reinstall::Writer> writer{WriterFactory()};

			// Step 4, write image.
			Logger::String{"Writing"}.info(name());
			dcntrl.run(dialogs.progress,[this,builder,writer](Dialog::Progress &dialog) {
				dialog.title(dialogs.title);
				dialog.message(_("Writing"));
				write(dialog,builder,writer);
				return 0;
			});

			dialogs.success.run();

		} catch(const std::exception &e) {

			dialogs.failed.run(e.what());

		} catch(...) {

			dialogs.failed.run(_("Unexpected error"));

		}

	}

 }

