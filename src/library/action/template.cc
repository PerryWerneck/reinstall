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
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	Action::Template::Template(const pugi::xml_node &node)
		: Template{
			Quark{node,"name"}.c_str(),
			Quark{node,"url"}.c_str(),
			Quark{node,"path"}.c_str()
		} {

		script = node.attribute("script").as_bool(script);
		const char *sMarker = node.attribute("marker").as_string(((std::string) Config::Value<String>("template","marker","$")).c_str());

		if(strlen(sMarker) > 1 || !sMarker[0]) {
			throw runtime_error("Marker attribute is invalid");
		}

		this->marker = sMarker[0];

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
			debug("Template already saved on '",filename,"'");
			return;
		}

		Dialog::Progress &progress = Dialog::Progress::getInstance();
		auto worker = Udjat::Protocol::WorkerFactory(String{this->url}.expand(object).c_str());

		progress.set_url(worker->url().c_str());
		Udjat::String contents = worker->get([&progress](double current, double total){
			progress.set_progress(current,total);
			return true;
		});

		// Expand ${} values using object.
		contents.expand(marker,object,true,true);

		debug("Marker = '",string{marker},"' \n",contents,"\n");

		// Save to temporary.
		debug("Saving template");
		filename = Udjat::File::save(contents.c_str());

		{
			mode_t mode = 0644;

			if(script) {
				Logger::String{"Template is script, using exec permission"}.trace(name);
				mode = 0755;
			} else {
				Logger::String{"Template is not script, using standard file permission"}.trace(name);
				mode = 0644;
			}

			if(chmod(filename.c_str(),mode) < 0) {
				throw system_error(errno,system_category(),_("Cant update template permissions"));
			}

		}
	}

	bool Action::Template::test(const char *isopath) const noexcept {

		if(this->path && *this->path) {
			return false;
		}

		if(*name == '/') {
			return strcmp(name,isopath) == 0;
		}

		const char *ptr = strrchr(isopath,'/');
		if(ptr) {
			ptr++;
		} else {
			ptr = isopath;
		}

		return strcmp(name,ptr) == 0;
	}

	void Action::Template::replace(const char *path) const {

		if(filename.empty()) {
			throw runtime_error(_("Template was not loaded"));
		}

		Logger::String{
			"Saving template '", name, "' as '", path, "'"
		}.info("template");

		Udjat::File::copy(filename.c_str(),path);

	}

	void Action::Template::apply(Source &source) {

		Logger::String{
			"Replacing file '", source.path, "' with '", name, "' template"
		}.trace("template");

		source.set_filename(this->filename.c_str());

	}

 }
