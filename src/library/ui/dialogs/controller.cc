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
  * @brief Implements abstract dialog controller.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/ui/dialog.h>
 #include <udjat/ui/dialogs/progress.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <vector>
 #include <iostream>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	Dialog::Controller * Dialog::Controller::cntrl = nullptr;

	Dialog::Controller::Controller() {

		if(cntrl) {
			throw logic_error("Dialog controller is already selected");
		}

		cntrl = this;
	}

	Dialog::Controller::~Controller() {

		if(cntrl == this) {
			cntrl = nullptr;
		}

	}

	Dialog::Controller & Dialog::Controller::instance() {

		if(!cntrl) {
			throw logic_error("Dialog controller was not selected");
		}

		return *cntrl;

	}

	void Dialog::Controller::quit(int) {
	}

	int Dialog::Controller::run(const Dialog &, const std::vector<Button> &) {
		errno = ENOTSUP;
		return -1;
	}

	int Dialog::Controller::run(const Dialog &dialog, const std::function<int(Progress &progress)> &task) {
		Dialog::Progress progress;
		progress.set(dialog);
		return task(progress);
	}

 }
