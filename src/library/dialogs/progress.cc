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
 #include <reinstall/dialogs.h>
 #include <iostream>
 #include <udjat/tools/intl.h>

 using namespace std;

 namespace Reinstall {

	namespace Dialog {

		void Progress::set(const Action &action) {
			set_icon_name(action.get_icon_name());
			set_title(action.get_label().c_str());
			set_step("");
		}

		Progress::~Progress() {
		}

		Progress & Progress::getInstance() {
			Progress *instance = dynamic_cast<Progress *>(&Window::getInstance());
			if(!instance) {
				throw runtime_error(_("No progress dialog available"));
			}
			return *instance;
		}

		void Progress::update(double current, double total) {
		}

		void Progress::count(size_t count, size_t total) {
		}

		void Progress::set_title(const char *title) {
		}

		void Progress::set_sub_title(const char *sub_title) {
		}

		void Progress::set_step(const char *step) {
		}

		void Progress::set_icon_name(const char *icon_name) {
		}

	}
 }
