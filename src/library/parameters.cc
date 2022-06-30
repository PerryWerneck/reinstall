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
 #include <reinstall/value.h>
 #include <stdexcept>
 #include <string>

 using namespace std;

 namespace Reinstall {

	Parameters::Parameters(const pugi::xml_node &node, const char *tagname) {

		for(pugi::xml_node nd = node; nd; nd = nd.parent()) {

			for(pugi::xml_node child = nd.child(tagname); child; child = child.next_sibling(tagname)) {

				for(pugi::xml_node value = child.child("value"); value; value = value.next_sibling("value")) {

					string name{value.attribute("name").as_string()};
					if(!name.empty()) {

						if(values.find(name) == values.end()) {
							values.insert(make_pair(name,Parameters::Value(value)));
						}

					}

				}

			}

		}

	}

	Parameters::~Parameters() {
	}

	/// @brief Has any value?
	bool Parameters::isNull() const {
		return false;
	}

	/// @brief Navigate from all values.
	void Parameters::for_each(const std::function<void(const char *name, const Udjat::Value &value)> &call) const {

		for(auto value = values.begin(); value != values.end(); value++) {


		}

	}

	/// @brief Clear contents, set value type.
	Udjat::Value & Parameters::reset(const Type type) {
		throw runtime_error("Unsupported request");
	}

	/// @brief Convert value to 'object' and insert child.
	Udjat::Value & Parameters::set(const Udjat::Value &value) {
		throw runtime_error("Unsupported request");
	}

 }
