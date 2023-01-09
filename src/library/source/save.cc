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

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	void Source::save() {

		debug("source-name=",name()," url=",this->url);

		if(!this->url[0]) {
			error() << "Cant save source with empty URL" << endl;
			throw runtime_error(_("Unable to get source with an empty URL"));
		}

		if(this->url[0] == '/') {
			error() << "Cant save source with relative URL '" << this->url << "'" << endl;
			throw runtime_error(_("Unable to get source with relative URL"));
		}

		if(!filenames.saved.empty()) {
			// Already downloaded, return
			warning() << "Already downloaded, ignoring request" << endl;
			return;
		}

		Dialog::Progress &progress = Dialog::Progress::getInstance();

		auto worker = Protocol::WorkerFactory(this->url);

		if(message && *message) {
			progress.set_sub_title(message);
		}
		progress.set_url(worker->url().c_str());

		if(!filenames.defined.empty()) {

			// Download URL to 'filename'
			worker->save(filenames.defined.c_str(),[&progress](double current, double total){
				progress.set_progress(current,total);
				return true;
			},true);

			filenames.saved = filenames.defined;

		} else if(filenames.temp.empty()) {

			// Download to temporary file.
			filenames.temp = worker->save([&progress](double current, double total){
				progress.set_progress(current,total);
				return true;
			});

			filenames.saved = filenames.temp;

		}

	}

 }
