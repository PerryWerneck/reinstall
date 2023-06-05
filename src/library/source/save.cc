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
 #include <reinstall/source.h>
 #include <reinstall/builder.h>
 #include <reinstall/dialogs.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/file.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	const char * Source::filename(bool rw) {

		if(!rw) {
			return filenames.saved.c_str();
		}

		if(filenames.temp.empty()) {

			// Will change contents, create a temporary file name.
			filenames.temp = File::Temporary::create();
			filenames.saved.clear();
			save(filenames.temp.c_str());

		}

		return filenames.saved.c_str();

	}

	void Source::save(const std::function<void(const void *buf, size_t length)> &write) {
		throw runtime_error("Custom saver is unsupported for this source");
	}

 	void Source::save(const char *filename) {

		if(!filenames.saved.empty()) {
			warning() << "Already downloaded" << endl;
			return;
		}

		if(!this->url[0]) {
			error() << "Cant save source with empty URL" << endl;
			throw runtime_error(_("Unable to get source with an empty URL"));
		}

		if(this->url[0] == '/') {
			error() << "Cant save source with relative URL '" << this->url << "'" << endl;
			throw runtime_error(_("Unable to get source with relative URL"));
		}

 		filenames.saved = filename;

		Dialog::Progress &progress = Dialog::Progress::getInstance();

		if(message && *message) {
			progress.set_sub_title(message);
		}

		progress.set_url(url);

 		if(strncasecmp(url,"file://",7) == 0) {

			// It's a file, just copy it.
			Udjat::File::copy(Udjat::URL{url}.ComponentsFactory().path.c_str(),filename,
			[&progress](double current, double total){
				progress.set_progress(current,total);
				return true;
			},true);

 		} else {

			// Not a file, download it.
			Protocol::WorkerFactory(this->url)->save(filename,[&progress](double current, double total){
				progress.set_progress(current,total);
				return true;
			},true);

 		}



 	}

	void Source::save() {

		if(!filenames.saved.empty()) {
			warning() << "Already downloaded" << endl;
			return;
		}

		if(!this->url[0]) {
			error() << "Cant save source with empty URL" << endl;
			throw runtime_error(_("Unable to get source with an empty URL"));
		}

		if(this->url[0] == '/') {
			error() << "Cant save source with relative URL '" << this->url << "'" << endl;
			throw runtime_error(_("Unable to get source with relative URL"));
		}

		if(strncasecmp(url,"file://",7) == 0) {
			filenames.saved = (url + 7);
			return;
		}

		Dialog::Progress &progress = Dialog::Progress::getInstance();

		auto worker = Protocol::WorkerFactory(this->url);

		if(message && *message) {
			progress.set_sub_title(message);
		}
		progress.set_url(worker->url().c_str());

		// Download to temporary file.
		filenames.temp = File::Temporary::create();
		save(filenames.temp.c_str());
		filenames.saved = filenames.temp;

	}

 }
