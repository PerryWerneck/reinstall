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
  * @brief Implements abstract menu item.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/ui/menu.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>

 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	Menu::Item::Item(const XML::Node &node) {
		Controller::instance().push_back(this,node);
	}

	Menu::Item::~Item() {
		Controller::instance().remove(this);
	}

	void Menu::Item::activate() {
		throw logic_error(_("This menu option cant be activated"));
	}

	void Menu::Item::set(const ActivationType type) noexcept {
		Controller::instance().set(this,type);
	}


 }