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
 #include <reinstall/worker.h>
 #include <reinstall/dialogs.h>
 #include <vector>
 #include <udjat/tools/intl.h>

 namespace Reinstall {

	Worker::Worker() {
	}

	void Worker::push_back(Source UDJAT_UNUSED(&source)) {
	}

	void Worker::pre(Action &action) {

		// Get folder contents.
		Dialog::Progress::getInstance().set_title(_("Getting file lists"));
		action.load();

		// Apply templates.
		Dialog::Progress::getInstance().set_title(_("Checking for templates"));
		action.applyTemplates();

		// Download files.
		Dialog::Progress::getInstance().set_title(_("Getting required files"));
		size_t total = action.source_count();
		size_t current = 0;
		action.for_each([this,&current,total](Source &source) {
			Dialog::Progress::getInstance().set_count(++current,total);
			push_back(source);
		});
		Dialog::Progress::getInstance().set_count(0,0);

	}

	void Worker::apply(Action &action) {
	}

	void Worker::post(Action &action) {
	}

	void Worker::burn(std::shared_ptr<Reinstall::Writer> UDJAT_UNUSED(writer)) {
	}

 }

