/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <libreinstall/source.h>
 #include <reinstall/dialogs/progress.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/file/temporary.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/http/mimetype.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 #include <fcntl.h>
 #include <stdexcept>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	void Source::prepare(std::set<std::shared_ptr<File>> &files) {

		/// @brief 'writer' for local files.
		class Local : public Source::File {
		private:
			const Udjat::File::Path from;

		public:
			Local(const Udjat::File::Path &f, const std::string &to) : Source::File{to}, from{f} {
			}

			void save(const std::function<void(unsigned long long offset, unsigned long long total, const void *buf, size_t length)> &writer) const override {
				from.save(writer);
			}

		};

		// TODO: File source saving url to original source local file. Will be used to recover local repository.

		/// @brief File source using URL to download to temp file.
		class Remote : public Source::File, private Udjat::File::Temporary {
		private:
			std::string url;

		public:

			Remote(const std::string &u, const std::string &to) : Source::File{to}, url{u} {
			}

			void save(const std::function<void(unsigned long long offset, unsigned long long total, const void *buf, size_t length)> &writer) const override {
				Udjat::File::Handler::save(writer);
			}

		};

		Dialog::Progress &progress = Dialog::Progress::getInstance();

		if(local()) {

			progress.set_url(path.local);
			Logger::String{"Getting file list from ",path.local}.trace(name);

			size_t szpath = strlen(path.local);

			Udjat::File::Path{path.local}.for_each([this,szpath,&files](const Udjat::File::Path &path){

				string target{imgpath};
				target += (path.c_str()+szpath);
				debug(target);

				files.emplace(make_shared<Local>(path,target));

				return false;

			},true);


			return;
		}

		// Base URL.
		std::string url{path.remote};

		// Do we have slp? If yes got URL using it.

		// Get files from URL.
		if(url[url.size()-1] == '/') {

			// It's a folder, get contents.
			auto worker = Protocol::WorkerFactory(url.c_str());
			worker->mimetype(MimeType::json);

			Logger::String{"Getting file list for ",worker->url().c_str()}.info(name);
			progress.set_url(worker->url().c_str());

			auto index = worker->get([&progress](double current, double total){
				progress.set_progress(current,total);
				return true;
			});

			debug(worker->header("Content-Type").value());
			debug(worker->header("Server").value());

			debug("index=\n",index,"\n");

		}

		throw runtime_error("Still incomplete");
	}

 }
