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
 #include <udjat/ui/dialogs/popup.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/intl.h>
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

	int Dialog::Controller::run(const std::function<int(Progress &progress)> &task) {
		return run(Dialog{},task);
	}

	int Dialog::Controller::run(const Dialog &dialog, const std::vector<Button> &) {
		debug(__FUNCTION__);
		Logger::String{"The selected backend cant manage dialogs with buttons, assuming '0' for '",dialog.message,"'"}.warning("ui");
		if(dialog.secondary && *dialog.secondary) {
			Logger::String{dialog.secondary}.warning("ui");
		}
		errno = ENOTSUP;
		return 0;
	}

	int Dialog::Controller::run(const Dialog &dialog, const std::function<int(Progress &progress)> &task) {
		debug(__FUNCTION__);
		Logger::String{"The selected backend cant manage progress dialogs, using default for '",dialog.message,"'"}.warning("ui");
		Dialog::Progress progress;
		progress.set(dialog);
		return task(progress);
	}

	int Dialog::Controller::run(const Dialog &dialog, const std::function<int(Popup &popup)> &task, const std::vector<Button> &) {
		debug(__FUNCTION__);
		Logger::String{"The selected backend cant manage popup dialogs, using default for '",dialog.message,"'"}.warning("ui");
		Dialog::Popup popup;
		popup.set(dialog);
		return task(popup);
	}

	bool Dialog::Controller::confirm(const Dialog &dialog, const char *yes, const char *no) {

		debug("------------------");

		if(!yes) {
			yes = _("_Yes");
		}

		if(!no) {
			no = _("_No");
		}

		vector<Button> buttons = {
			{ 1, no	 },
			{ 0, yes },
		};

		return run(dialog,buttons) == 0;

	}


 }
