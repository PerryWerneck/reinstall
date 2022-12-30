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
 #include <udjat/tools/logger.h>

 using namespace std;
 using namespace Udjat;

 Dialog::TaskRunner::TaskRunner(Gtk::Window &parent, const char *message, bool markup)
	: Gtk::MessageDialog{parent,message,markup,Gtk::MESSAGE_INFO,Gtk::BUTTONS_OK,true} {
 }

 void Dialog::TaskRunner::on_response(int response_id) {
 	debug("Response=",response_id);
 }

 int Dialog::TaskRunner::push(const std::function<int()> &callback) {

	int rc = -1;

	Udjat::ThreadPool::getInstance().push([this,&callback,&rc](){

		debug("Background task begin");

		try {

			rc = callback();

		} catch(std::exception &e) {
			cerr << e.what() << endl;
			rc = -1;
		} catch(...) {
			cerr << "Unexpected error running background task" << endl;
			rc = -1;
		}

		debug("Background task end");

	});

	run();

	return rc;
 }

 void dismiss(int response_id);

 void Dialog::TaskRunner::set_title(const char *text, bool markup) {

 	auto str = make_shared<string>(text);
	Glib::signal_idle().connect([this,str,markup](){
		this->set_message(str->c_str(),markup);
		return 0;
	});

 }

 void Dialog::TaskRunner::set_sub_title(const char *text, bool markup) {

 	auto str = make_shared<string>(text);
	Glib::signal_idle().connect([this,str,markup](){
		this->set_secondary_text(str->c_str(),markup);
		return 0;
	});

 }

 // void 	add_action_widget (Widget& child, int response_id)
