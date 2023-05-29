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
 #include <reinstall/builder.h>
 #include <reinstall/actions/fsbuilder.h>
 #include <reinstall/diskimage.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/quark.h>
 #include <cstdio>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	FSBuilder::FSBuilder(const pugi::xml_node &node, const char *icon_name)
		: Reinstall::Action(node,icon_name), imglen{getImageSize(node)}, fsname{Quark{node,"filesystem","fat32"}.c_str()} {
		if(!imglen) {
			throw runtime_error("Required attribute 'size' is missing");
		}

	}

	FSBuilder::~FSBuilder() {
	}

	std::shared_ptr<Reinstall::Builder> FSBuilder::BuilderFactory() {

		class Builder : public Reinstall::Builder, private Disk::Image {
		private:
			std::string filename;

		public:
			Builder(const std::string &fname, const char *fsname, unsigned long long length)
				: Disk::Image{
					fname.c_str(),
					fsname,
					length
				}, filename{fname} {

			}

			virtual ~Builder() {
				remove(filename.c_str());
			}

			void pre(const Action &action) override {
			}

			void post(const Action &action) override {
			}

			void build(Action &action) override {
			}

			bool apply(Source &source) override {
				Disk::Image::insert(source);
				return true;
			}

			std::shared_ptr<Writer> burn(std::shared_ptr<Writer> writer) override {

				throw runtime_error("Incomplete");

			}

		};


		return make_shared<Builder>(File::Temporary::create(),fsname,imglen);

	}

	std::shared_ptr<Reinstall::Writer> FSBuilder::WriterFactory() {
		debug("Returning USB writer");
		return Reinstall::Writer::USBWriterFactory(*this);
	}

	bool FSBuilder::interact() {
		return true;
	}

 }

