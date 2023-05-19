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
 #include <udjat/tools/logger.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	namespace Dialog {

		Progress::~Progress() {
		}

		Progress & Progress::getInstance() {
			Progress *instance = dynamic_cast<Progress *>(&Window::getInstance());
			if(!instance) {
				throw runtime_error(_("No active progress dialog"));
			}
			return *instance;
		}

		void Progress::set_progress(double current, double total) {

			values.changed = true;

			if(total > current && total > 1.0) {
				values.current = current;
				values.total = total;
			} else {
				total = current = 0.0;
			}
		}

		void Progress::set_count(size_t current, size_t total) {
			if(total && current) {
				set_step(Logger::Message{_("{} of {}"),current,total}.c_str());
			} else {
				set_step("");
			}
		}

		void Progress::set_title(const char *title) {
			debug("title: ",title);
		}

		void Progress::set_sub_title(const char *sub_title) {
			debug("sub-title: ",sub_title);
		}

		void Progress::set_url(const char *url) {
			set_sub_title(url);
		}

		void Progress::pulse() {
		}

		void Progress::set_step(const char *step) {
		}

		void Progress::set_icon_name(const char *icon_name) {
		}

	}
 }
