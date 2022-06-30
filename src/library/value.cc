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
 #include <udjat/tools/string.h>

 using namespace std;

 namespace Reinstall {

	Parameters::Value::Value(const pugi::xml_node &node) : std::string(Udjat::String(node.attribute("value").as_string()).expand(node).c_str()) {
	}

	Parameters::Value::~Value() {
	}

	bool Parameters::Value::isNull() const {
		return empty();
	}

	Udjat::Value & Parameters::Value::reset(const Type type) {
		clear();
		return *this;
	}

	Udjat::Value & Parameters::Value::set(const Udjat::Value &value) {
		throw runtime_error("Invalid operation");
	}

	Udjat::Value & Parameters::Value::set(const char *value, const Type type) {
		this->assign(value);
		return *this;
	}

	const Udjat::Value & Parameters::Value::get(std::string &value) const {
		value = c_str();
		return *this;
	}

 }
