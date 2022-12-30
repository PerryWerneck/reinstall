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
 #include <udjat/defs.h>
 #include <gtkmm.h>
 #include <glibmm/i18n.h>
 #include <reinstall/dialogs/taskrunner.h>
 #include <private/dialogs.h>
 #include <udjat/tools/threadpool.h>
 #include <iostream>

 using namespace std;
 using namespace Udjat;

 Dialog::TaskRunner::TaskRunner() {
 }

 int Dialog::TaskRunner::push(const std::function<int()> &callback) {

	Udjat::ThreadPool::getInstance().push([this,&callback](){

		int rc = -1;

		try {

			rc = callback();

		} catch((std::exception &e) {
			cerr << e.what() << endl;
			rc = -1;
		} catch(..) {
			cerr << "Unexpected error running background task" << endl;
			rc = -1;
		}

		Glib::signal_idle().connect([this,rc](){
			this->response(rc);
		});

	});

	show_all();
	return (int) run();

 }

 void dismiss(int response_id);

 void Dialog::TaskRunner::set_title(const char *markup) {

 	auto str = make_shared<string>(markup);
	Glib::signal_idle().connect([this,str](){
		this->set_message(str.c_str(),true);
	});

 }

 void Dialog::TaskRunner::set_sub_title(const char *markup) {

	auto str = make_shared<string>(markup);
	Glib::signal_idle().connect([this,str](){
		this->set_secondary_text(str.c_str(),true);
	});

 }
