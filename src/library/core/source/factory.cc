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
  * @brief Implements source factory.
  */

 #include <config.h>
 #include <memory>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/logger.h>

 #include <libreinstall/source.h>
 #include <libreinstall/sources/zip.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	std::shared_ptr<Source> Source::factory(const Udjat::XML::Node &node) {

		debug("NAME=",node.attribute("name").as_string("???"));

		switch(XML::StringFactory(node,"type","value","default").select("default","zip")) {
		case 0: // Default
			return make_shared<Source>(node);

		case 1:	// zip
			return make_shared<ZipSource>(node);

		default:
			throw runtime_error("Unexpected repository type");
		}

	}

 }
