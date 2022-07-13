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
 #include <udjat/tools/quark.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/protocol.h>
 #include <reinstall/dialogs.h>
 #include <udjat/tools/file.h>
 #include <iostream>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	Action::Template::Template(const pugi::xml_node &node) : name(Quark(node,"name").c_str()), url(Quark(node,"url").c_str()) {
	}

	Action::Template::~Template() {
		if(!filename.empty()) {
			if(remove(filename.c_str()) != 0) {
				cerr << "tempfile\tError removing '" << filename << "'" << endl;
			}
		}
	}

	void Action::Template::load(const Udjat::Object &object) {

		if(!filename.empty()) {
			return;
		}

		Dialog::Progress &progress = Dialog::Progress::getInstance();
		auto worker = Udjat::Protocol::WorkerFactory(this->url);

		progress.set(worker->url().c_str());
		Udjat::String contents = worker->get([&progress](double current, double total){
			progress.update(current,total);
			return true;
		});

		// Expand ${} values using object.
		contents.expand(object);

		// Save to temporary.
		filename = Udjat::File::save(contents.c_str());

	}

	bool Action::Template::test(const char *path) const noexcept {

		if(*name == '/') {
			return strcmp(name,path) == 0;
		}

		const char *ptr = strrchr(path,'/');
		if(ptr) {
			ptr++;
		} else {
			ptr = path;
		}

		return strcmp(name,ptr) == 0;
	}


	void Action::Template::apply(Source &source) {

		cout << "templates\tReplacing file '" << source.path << "' with '" << name << "' template" << endl;
		source.filename = this->filename.c_str();

	}

 }
