/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2022 Perry Werneck <perry.werneck@gmail.com>
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
 #include <reinstall/userinterface.h>
 #include <udjat/tools/logger.h>
 #include <reinstall/object.h>
 #include <stdexcept>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	UserInterface * UserInterface::instance = nullptr;

	UserInterface & UserInterface::getInstance() {

		if(instance) {
			return *instance;
		}

		throw runtime_error("No user interface");

	}

	UserInterface::UserInterface() {
		if(instance) {
			throw runtime_error("User interface is already set");
		}
		instance = this;
	}

	UserInterface::~UserInterface() {
		if(instance == this) {
			instance = nullptr;
		}
	}

	std::string UserInterface::FilenameFactory(const char *, const char *, const char *, const char *, bool) {
		throw system_error(ENOTSUP,system_category(),"Cant get filename");
	}

 }
