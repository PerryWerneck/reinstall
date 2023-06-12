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
 #include <udjat/tools/xml.h>
 #include <udjat/tools/file/temporary.h>
 #include <udjat/tools/file/handler.h>
 #include <udjat/tools/file.h>
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

	void Template::apply(const Udjat::Abstract::Object &object, std::set<std::shared_ptr<Reinstall::Source::File>> &files) {

		class Parsed : public Source::File {
		private:
			const Udjat::File::Path from;

		public:
			Parsed(const char *filename, const char *to) : Source::File{to}, from{filename} {
				Logger::String{filename," -> ",to}.write(Logger::Debug,"template");
			}

			virtual ~Parsed() {
				if(unlink(from.c_str()) < 0) {
					Logger::String{"Cant remove '",from.c_str(),"': ",strerror(errno)}.warning("template");
				}
			}

			bool remote() const noexcept override {
				return false;
			}

			const char * path() const noexcept override {
				return from.c_str();
			}

			void save(const std::function<void(unsigned long long offset, unsigned long long total, const void *buf, size_t length)> &writer) const override {
				from.save(writer);
			}

		};

		/// @brief New sources, with prepared templates.
		vector<shared_ptr<Parsed>> updated;

		// Search for templates, when found, remove foi files, append on updated.
		for(auto it = files.begin(); it != files.end();) {

			std::shared_ptr<Reinstall::Source::File> file = *it; // Convenience.

			if(test(file->c_str())) {

				Logger::String{"Applying template on '",file->c_str(),"'"}.trace(this->name());

				// Match template, Download source to string.
				std::stringstream stream;

				file->save([&stream](unsigned long long, unsigned long long, const void *buf, size_t length){
					stream.write((const char *) buf, length);
				});

				// Parse
				Udjat::String str{stream.str()};
				str.expand(object,true,true);

				// Create new source using parsed string.
				string filename{Udjat::File::Temporary::create()};
				Udjat::File::Handler{filename.c_str(),true}.write(str.c_str());

				// Add the parsed source in the list.
				updated.push_back(make_shared<Parsed>(filename.c_str(),file->c_str()));

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
