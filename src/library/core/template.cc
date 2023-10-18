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
 #include <libreinstall/source.h>
 #include <udjat/ui/dialogs/progress.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/file/temporary.h>
 #include <udjat/tools/file/handler.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>
 #include <vector>
 #include <memory>
 #include <set>
 #include <sstream>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace Udjat;
 using namespace std;

 using Progress = Udjat::Dialog::Progress;

 namespace Reinstall {

	static char get_marker(const Udjat::XML::Node &node) {
		const char *marker = node.attribute("marker").as_string(((std::string) Config::Value<String>("template","marker","$")).c_str());

		if(strlen(marker) > 1 || !marker[0]) {
			throw runtime_error("Marker attribute is invalid");
		}

		return marker[0];
	}

	Template::Template(const Udjat::XML::Node &node) :
		Udjat::NamedObject{node},
		filter{Udjat::XML::QuarkFactory(node,"filter").c_str()},
		url{Udjat::XML::QuarkFactory(node,"url").c_str()},
		filename{Udjat::XML::QuarkFactory(node,"path").c_str()},
		marker{get_marker(node)},
		escape_values{getAttribute(node,"template","escape-control-characters",strncasecmp(name(),"grub",4) == 0)},
		escape_chars{Udjat::XML::QuarkFactory(node,"filter","&").c_str()} {

		if(!(filter && *filter)) {
			filter = strrchr(url,'/');
			if(filter) {
				filter++;
			} else {
				throw runtime_error("Required attribute 'filter' is missing or invalid");
			}
		}

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

	Udjat::String Template::apply(const Udjat::Abstract::Object &object) const {

		Progress &progress{Progress::instance()};

		auto worker = Protocol::WorkerFactory(String{this->url}.expand(object).c_str());

		Logger::String{"Loading ",worker->url().c_str()}.write(Logger::Trace,"template");
		progress.url(worker->url().c_str());

		String text = worker->get([&progress](double current, double total){
			progress.progress(current,total);
			return true;
		});

		/// TODO: expand escaped values.
		text.expand(object);

		if(Logger::enabled(Logger::Debug)) {
			Logger::String{"New contents:\n",text.c_str(),"\n"}.write(Logger::Debug,name());
		}

		return text;

	}

	std::shared_ptr<Reinstall::Source::File> Template::SourceFactory(const Udjat::Abstract::Object &object, const char *path) const {

		/// @brief Template text with resolved ${}.
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

		String text{this->apply(object)};

		return make_shared<Parsed>(
					text,
					path
				);
	}

 }
