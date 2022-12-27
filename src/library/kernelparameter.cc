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
 #include <reinstall/action.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/intl.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	Action::KernelParameter::KernelParameter(const pugi::xml_node &node) : nm(Quark(node.attribute("name").as_string()).c_str()) {

		if(!name()[0]) {
			throw runtime_error(_("Unnamed kernel parameter"));
		}

		pugi::xml_attribute attribute;

		static const struct Args {
			Type type;
			const char *name;
		} args[] = {
			{ Value, 		"value"			},
			{ Url, 			"url"			}
		};

		type = Invalid;
		for(size_t ix=0; ix < (sizeof(args)/sizeof(args[0])); ix++) {

			attribute = node.attribute(args[ix].name);
			if(attribute) {
				type = args[ix].type;
				vl = attribute.as_string();
				vl.expand(node,false);
				break;
			}

		}

		if(type == Invalid) {
			throw runtime_error(_("Invalid kernel parameter value type"));
		}

	}

	Action::KernelParameter::~KernelParameter() {
	}

	void Action::KernelParameter::set(const Action &action) {

		if(type == Url) {

			if(vl.empty() || vl[0] == '/') {

				// Fix URL with repository.
				URL url(action.repository(repository)->url(false));

				if(!vl.empty()) {
					vl.expand(action,false);
					url += vl.c_str();
				}

				String value{url.c_str()};
				value.expand(action,true);

				action.info() << "Kernel parameter '" << vl << "' expanded to " << value << endl;
				vl = value;

			}

		} else {

			vl.expand(action,true);

		}

	}


 }
