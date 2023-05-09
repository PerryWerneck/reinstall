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
 #include <reinstall/defs.h>
 #include <reinstall/action.h>
 #include <reinstall/sources/efiboot.h>
 #include <reinstall/source.h>
 #include <reinstall/dialogs.h>
 #include <reinstall/diskimage.h>
 #include <pugixml.hpp>
 #include <udjat/tools/intl.h>
 #include <iostream>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/configuration.h>
 #include <ctype.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	std::shared_ptr<EFIBootImage> EFIBootImage::factory(const pugi::xml_node &node) {


		// Create default object.
		return make_shared<EFIBootImage>(node);

	}

	EFIBootImage::EFIBootImage(const pugi::xml_node &node) : NamedObject{node} {

		options.enabled = getAttribute(
						node,
						"efi-boot-image",
						"enabled",
						options.enabled
					);

		options.path = getAttribute(
						node,
						"efi-boot-image",
						"path",
						options.path
		);


		// Get target image size.
		{
			Udjat::String attr {node.attribute("size").as_string()};
			attr.strip();

			if(!attr.empty()) {

				options.size = 0;

				const char *ptr = attr.c_str();
				while(*ptr && isdigit(*ptr)) {
					options.size *= 10;
					options.size += (*ptr - '0');
					ptr++;
				}

				while(*ptr && isspace(*ptr)) {
					ptr++;
				}

				if(*ptr) {
					static const char *units[] = { "B", "KB", "MB", "GB" };

					bool found = false;
					for(const char *unit : units) {
						if(!strcasecmp(ptr,unit)) {
							found = true;
							break;
						}
						options.size *= 1024;
					}

					if(!found) {
						throw runtime_error(Logger::String{"Unexpected size unit: '",ptr,"'"});
					}

				}

				Logger::String{"Will build a ",String{}.set_byte((unsigned long long) options.size)," boot image"}.trace(name());

			}


		}

	}

	void EFIBootImage::build(Reinstall::Action &action) {

		debug("-----------------------------------------------------------------");

		if(options.size) {

			Dialog::Progress &dialog = Dialog::Progress::getInstance();

			dialog.set_sub_title(_("Building EFI Boot image"));

			// Create disk
			Disk::Image disk{
				File::Temporary::create().c_str(),
				Config::Value<string>{"efi","filesystem","vfat"}.c_str(),
				options.size
			};

			// Copy EFI files
			action.for_each([&disk,&dialog](Source &source){

				if(strncasecmp(source.path,"efi/",4)) {
					return;
				}

				dialog.set_url(source.path);
				disk.copy(source.filename(),source.path);

			});


		}

	}

 }
