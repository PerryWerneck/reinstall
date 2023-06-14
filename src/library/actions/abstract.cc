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
 #include <libreinstall/action.h>
 #include <libreinstall/dialogs/taskrunner.h>
 #include <libreinstall/dialogs/progress.h>
 #include <stdexcept>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	Action *Action::selected = nullptr;
	Action *Action::def = nullptr;

	Action::Action(const XML::Node &node) : NamedObject{node} {

		if(getAttribute(node, "action-defaults", "default", false)) {
			Action::def = this;
		}

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
			for(const Reinstall::Source &source : sources) {
				source.prepare(files);
			}
		});

		if(!tmpls.empty()) {

			progress.run(_("Applying templates"),[this,&files](){
				for(const Reinstall::Template &tmpl : tmpls) {
					tmpl.apply(Udjat::Abstract::Object{},files);
				}
			});

		}

	}

	void Action::build(Dialog::Progress &progress, std::shared_ptr<Reinstall::Builder> builder, std::set<std::shared_ptr<Reinstall::Source::File>> &files) const {

		progress.run(_("Building image"),[this,builder,&files](){
			builder->pre();
			for(auto file : files) {
				builder->push_back(file);
			}
		});

		if(!tmpls.empty()) {

			progress.run(_("Applying templates"),[this,builder](){
				builder->push_back(*this,tmpls);
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

