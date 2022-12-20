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
 #include <private/dialogs.h>
 #include <iostream>

 using namespace Gtk;
 using namespace std;

 Dialog::Progress::Progress() {

	Gtk::Box &content_area = *get_content_area();

	content_area.set_border_width(12);
	content_area.set_spacing(12);

	content_area.set_homogeneous(false);
	content_area.set_orientation(ORIENTATION_VERTICAL);

	content_area.pack_start(widgets.title,false,false,0);

	widgets.progress.set_valign(ALIGN_CENTER);
	content_area.pack_start(widgets.progress,true,true,0);

	widgets.footer.pack_start(widgets.action,false,false,3);
	widgets.footer.pack_start(widgets.message,true,true,3);
	widgets.footer.pack_end(widgets.step,false,false,3);

	content_area.pack_end(widgets.footer,false,false,0);

	content_area.show_all();

	sigc::slot<bool()> slot = sigc::bind(sigc::mem_fun(*this, &::Dialog::Progress::on_timeout), 1);

	this->timer.connection = Glib::signal_timeout().connect(slot,100);

 }

 bool Dialog::Progress::on_timeout(int timer_number) {

	if(is_visible()) {
		if(timer.idle >= 100) {
			widgets.progress.pulse();
		} else {
			timer.idle++;
		}
	}

	return true;
 }


