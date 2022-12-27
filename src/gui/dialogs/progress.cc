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
 #include <udjat/tools/logger.h>

 using namespace Udjat;
 using namespace Gtk;
 using namespace std;

 Dialog::Progress::Progress() {

	Gtk::Box &content_area = *get_content_area();

	set_decorated(false);

	set_default_size(400,-1);

	get_style_context()->add_class("dialog-progress");
	content_area.get_style_context()->add_class("dialog-contents");
	content_area.set_border_width(12);
	content_area.set_spacing(12);

	content_area.set_homogeneous(false);
	content_area.set_orientation(ORIENTATION_VERTICAL);

	content_area.pack_start(widgets.title,false,false,0);

	widgets.progress.set_valign(ALIGN_CENTER);
	content_area.pack_start(widgets.progress,true,true,0);

	widgets.footer.pack_start(widgets.action,false,false,3);

	widgets.message.set_line_wrap(false);
	widgets.message.set_ellipsize(Pango::ELLIPSIZE_START);
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

 void Dialog::Progress::footer(bool enable) {
	if(enable) {
		widgets.footer.show_all();
	} else {
		widgets.footer.hide();
	}
 }

 void Dialog::Progress::set_parent(Gtk::Window &window) {
	set_modal(true);
	set_transient_for(window);

 }

 bool Dialog::Progress::on_dismiss(int response_id) {
 	response(response_id);
 	return false;
 }

 void Dialog::Progress::dismiss(int response_id) {
	Glib::signal_idle().connect(sigc::bind<1>( sigc::mem_fun(this,&::Dialog::Progress::on_dismiss),response_id) );
 }

 void Dialog::Progress::show() {

 	Glib::signal_idle().connect([this](){
		Gtk::Dialog::show();
		return 0;
 	});

 }

 void Dialog::Progress::hide() {

 	Glib::signal_idle().connect([this](){
		Gtk::Dialog::hide();
		return 0;
 	});

 }

 void Dialog::Progress::set(const char *message)  {

	auto str = make_shared<string>(message);

 	Glib::signal_idle().connect([this,str](){
		widgets.title.set_text(str->c_str());
		return 0;
 	});

 }

 void Dialog::Progress::count(size_t count, size_t total)  {

 	Glib::signal_idle().connect([this,count,total](){

		return 0;
 	});

 }

 void Dialog::Progress::update(double current, double total)  {

 	Glib::signal_idle().connect([this,current,total](){

		if(total > current && total > 1) {
			timer.idle = 0;
			gdouble fraction = ((gdouble) current) / ((gdouble) total);
			widgets.progress.set_fraction(fraction);
		} else {
			widgets.step.set_text("");
		}

		return 0;

 	});

 }

 void Dialog::Progress::set(const Reinstall::Abstract::Object &object) {

 	Glib::signal_idle().connect([this,&object](){

		// object.set_dialog(*this);
		sub_title().set_text(_("Initializing"));
		action().set_text("");
		message().set_text("");
		step().set_text("");

		timer.idle = -1;

		/*
		if(object.icon && *object.icon) {

			// https://developer-old.gnome.org/gtkmm/stable/classGtk_1_1Image.html
			icon().set_from_icon_name(object.icon,Gtk::ICON_SIZE_DND);
			icon().show_all();

		} else {

			icon().hide();

		}
		*/

		return 0;

 	});

 }

