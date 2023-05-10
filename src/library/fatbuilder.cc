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
 #include <reinstall/actions/fatbuilder.h>
 #include <reinstall/diskimage.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/configuration.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	FatBuilder::FatBuilder(const pugi::xml_node &node, const char *icon_name)
		: Reinstall::Action(node,icon_name), imglen{getImageSize(node)} {

	}

	FatBuilder::~FatBuilder() {
	}

	std::shared_ptr<Reinstall::Builder> FatBuilder::BuilderFactory() {

		class Builder : public Reinstall::Builder, private Disk::Image {
		public:
			Builder(const char *filename, unsigned long long length)
				: Disk::Image{
					filename,
					Config::Value<string>{"fatbuilder","filesystem","fat32"}.c_str(),
					length
				} {
			}

			void pre(const Action &action) override {
			}

			void post(const Action &action) override {
			}

			void build(Action &action) override {
			}

			bool apply(Source &source) override {
				Disk::Image::insert(source);
			}

		};

		filename = File::Temporary::create();
		return make_shared<Builder>(filename.c_str(),imglen);

	}

	std::shared_ptr<Reinstall::Writer> FatBuilder::WriterFactory() {
		return Reinstall::Writer::USBWriterFactory(*this);
	}

	bool FatBuilder::interact() {
		return true;
	}

 }

