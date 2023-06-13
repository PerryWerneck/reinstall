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

 /**
  * @brief Implement template processing.
  */

 #include <config.h>
 #include <libreinstall/template.h>
 #include <reinstall/dialogs/progress.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/file/temporary.h>
 #include <udjat/tools/file/handler.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/logger.h>
 #include <vector>
 #include <memory>
 #include <set>
 #include <sstream>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	Template::Template(const Udjat::XML::Node &node) :
		Udjat::NamedObject{node},
		filter{Udjat::XML::QuarkFactory(node,"filter").c_str()},
		url{Udjat::XML::QuarkFactory(node,"url").c_str()},
		script{node.attribute("script").as_bool(false)},
		marker{Udjat::XML::QuarkFactory(node,"url").c_str()} {
	}

	bool Template::test(const char *path) const noexcept {

		if(!strcasecmp(path,this->filter)) {
			return true;
		}

		if(this->filter[0] != '/') {
			const char *ptr = strrchr(path,'/');
			if(ptr && strcasecmp(++ptr,this->filter) == 0) {
				return true;
			}
		}

		return false;
	}

	Udjat::String Template::get() const {

		Dialog::Progress &progress{Dialog::Progress::getInstance()};

		auto worker = Protocol::WorkerFactory(this->url);

		Logger::String{"Loading ",worker->url().c_str()}.write(Logger::Trace,"template");
		progress.set_url(worker->url().c_str());

		return worker->get([&progress](double current, double total){
			progress.set_progress(current,total);
			return true;
		});

	}

	void Template::apply(const Udjat::Abstract::Object &object, std::set<std::shared_ptr<Reinstall::Source::File>> &files) {

		class Parsed : public Source::File {
		private:
			const Udjat::String text;

		public:
			Parsed(const Udjat::String &t, const char *to) : Source::File{to}, text{t} {
			}

			virtual ~Parsed() {
			}

			bool remote() const noexcept override {
				return true;
			}

			const char * path() const override {
				throw runtime_error("Unexpected call to 'path' on template source");
			}

			void save(const std::function<void(unsigned long long offset, unsigned long long total, const void *buf, size_t length)> &writer) const override {
				writer(0,text.size(),text.c_str(),text.size());
			}

		};

		/// @brief New sources, with prepared templates.
		vector<shared_ptr<Parsed>> updated;

		// Search for templates, when found, remove foi files, append on updated.
		for(auto it = files.begin(); it != files.end();) {

			std::shared_ptr<Reinstall::Source::File> file = *it; // Convenience.

			if(test(file->c_str())) {

				Logger::String{"Apply ",this->url," on '",file->c_str()}.trace(this->name());

				// Parse
				updated.push_back(
					make_shared<Parsed>(
						this->get().expand(object,true,true),
						file->c_str()
					)
				);

				// And remove the old one.
				it = files.erase(it);

			} else {

				// No match, test the next one.
				++it;
			}

		}

		debug("file length: ",files.size());

		// Insert contents of 'updated' in files.
		for(auto source : updated) {
			Logger::String{"Replacing '", source->c_str(),"'"}.trace("template");
			files.insert(source);
		}

	}

 }
