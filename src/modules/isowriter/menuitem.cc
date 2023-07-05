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
  * @brief Implements isowriter menu activation.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/ui/dialog.h>

 #include <libreinstall/writer.h>

 #include "private.h"

 using namespace Udjat;

 void MenuItem::activate() {

	if(!dialogs.confirmation.confirm()) {
		Logger::String{"Cancel by user choice"}.info(name());
		return;
	}

	try {

		std::shared_ptr<Udjat::File::Handler> file;

		Dialog::Controller::instance().run(dialogs.progress,[this,&file](Dialog::Progress &dialog) {
			dialog.title(dialogs.title);
			file = get_file(dialog);
			return 0;
		});

		// Get writer.
		auto writer = Reinstall::Writer::factory(dialogs.title);

		Dialog::Controller::instance().run(dialogs.progress,[this,file,writer](Dialog::Progress &dialog) {
			dialog.title(dialogs.title);
			writer->write(dialog,*file);
			return 0;
		});

		dialogs.success.run();

	} catch(const std::exception &e) {

		Logger::String{e.what()}.error(name());
		dialogs.failed.run(e.what());

	} catch(...) {

		Logger::String{"Unexpected error"}.error(name());
		dialogs.failed.run(_("Unexpected error"));

	}

 }

