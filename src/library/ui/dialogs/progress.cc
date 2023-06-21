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
  * @brief Implement abstract progress dialog.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/ui/dialog.h>
 #include <udjat/ui/dialogs/progress.h>
 #include <udjat/tools/logger.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	Dialog::Progress * Dialog::Progress::current = nullptr;

	Dialog::Progress::Progress() : parent{current} {
		if(current) {
			current->hide();
		}
		current = this;
	}

	Dialog::Progress::~Progress() {

		if(current == this) {
			current = parent;
			if(current) {
				current->show();
			}
		} else {
			cerr << "dialog\tUnexpected dialog order" << endl;
		}

	}

	Dialog::Progress & Dialog::Progress::instance() {
		if(!current) {
			throw runtime_error("No active progress dialog");
		}
		return *current;
	}

	void Dialog::Progress::show() {
	}

	void Dialog::Progress::hide() {
	}

	void Dialog::Progress::title(const char *) {
	}

	void Dialog::Progress::message(const char *message) {
		if(message && *message) {
			Logger::String{message}.info("");
		}
	}

	std::string Dialog::Progress::message() const {
		return "";
	}

	void Dialog::Progress::secondary(const char *) {
	}

	void Dialog::Progress::icon(const char *) {
	}

	void Dialog::Progress::url(const char *url) {
		if(url && *url && Logger::enabled(Logger::Debug)) {
			Logger::String{url}.write(Logger::Debug,"");
		}
	}

	void Dialog::Progress::pulse() {
	}

	void Dialog::Progress::progress(double, double) {
	}

	void Dialog::Progress::count(size_t, size_t) {
	}

	void Dialog::Progress::run(const char *, const std::function<void()> &background_task) {
		background_task();
	}

 }



