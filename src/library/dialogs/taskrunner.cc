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
 #include <reinstall/dialogs.h>
 #include <reinstall/userinterface.h>
 #include <memory>
 #include <iostream>

 using namespace std;

 namespace Reinstall {

	std::shared_ptr<Dialog::TaskRunner> UserInterface::TaskRunnerFactory() {
		return make_shared<Dialog::TaskRunner>();
	}

	Dialog::TaskRunner::TaskRunner() {
	}

	Dialog::TaskRunner::~TaskRunner() {
	}

	void Dialog::TaskRunner::push(const std::function<void()> &callback) {
		callback();
	}

	void Dialog::TaskRunner::set_title(const char *markup) {
		cout << markup << endl;
	}

	void Dialog::TaskRunner::set_sub_title(const char *markup) {
		cout << markup << endl;
	}

	void Dialog::TaskRunner::add_button(const char *label, const std::function<void()> &callback) {
	}


 }
