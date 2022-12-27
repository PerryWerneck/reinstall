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

	set_default_size(500,-1);

	get_style_context()->add_class("dialog-progress");
	content_area.get_style_context()->add_class("dialog-contents");
	content_area.set_border_width(12);
	content_area.set_spacing(12);

	content_area.set_homogeneous(false);
	content_area.set_orientation(ORIENTATION_VERTICAL);

	widgets.title.get_style_context()->add_class("dialog-title");
	widgets.title.set_hexpand(true);
	widgets.title.set_vexpand(false);
	widgets.title.set_valign(ALIGN_START);
	widgets.title.set_line_wrap(false);
	widgets.title.set_ellipsize(Pango::ELLIPSIZE_START);
	widgets.header.attach(widgets.title,0,0,1,1);

	widgets.subtitle.get_style_context()->add_class("dialog-subtitle");
	widgets.subtitle.set_hexpand(true);
	widgets.subtitle.set_vexpand(false);
	widgets.subtitle.set_valign(ALIGN_START);
	widgets.subtitle.set_line_wrap(false);
	widgets.subtitle.set_ellipsize(Pango::ELLIPSIZE_START);
	widgets.header.attach(widgets.subtitle,0,1,1,1);

	widgets.icon.get_style_context()->add_class("dialog-icon");
	widgets.icon.set_hexpand(false);
	widgets.icon.set_vexpand(false);
	widgets.header.attach(widgets.icon,1,0,2,2);

	content_area.pack_start(widgets.header,false,false,0);

	widgets.progress.set_valign(ALIGN_CENTER);
	content_area.pack_start(widgets.progress,true,true,0);

	widgets.footer.get_style_context()->add_class("dialog-footer");
	widgets.footer.set_homogeneous(true);
	widgets.action.get_style_context()->add_class("dialog-action");
	widgets.step.get_style_context()->add_class("dialog-step");
	widgets.footer.pack_start(widgets.action,true,true,0);
	widgets.footer.pack_end(widgets.step,true,true,0);

	content_area.pack_end(widgets.footer,false,false,0);

	content_area.show_all();

	sigc::slot<bool()> slot = sigc::bind(sigc::mem_fun(*this, &::Dialog::Progress::on_timeout), 1);

	this->timer.connection = Glib::signal_timeout().connect(slot,100);

 }

 bool Dialog::Progress::on_timeout(int UDJAT_UNUSED(timer_number)) {

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

 void Dialog::Progress::set_title(const char *title)  {

	auto str = make_shared<string>(title);

 	Glib::signal_idle().connect([this,str](){
		widgets.title.set_text(str->c_str());
		return 0;
 	});

 }

 void Dialog::Progress::set_sub_title(const char *sub_title)  {

	auto str = make_shared<string>(sub_title);

 	Glib::signal_idle().connect([this,str](){
		if(str->empty()) {
			widgets.subtitle.hide();
		} else {
			widgets.subtitle.set_text(str->c_str());
			widgets.subtitle.show();
		}
		return 0;
 	});

 }

 void Dialog::Progress::set_step(const char *step)  {

	auto str = make_shared<string>(step);

 	Glib::signal_idle().connect([this,str](){
		widgets.step.set_text(str->c_str());
		return 0;
 	});

 }

 void Dialog::Progress::set_icon_name(const char *icon_name) {

	auto str = make_shared<string>(icon_name);

 	Glib::signal_idle().connect([this,str](){

		if(str->empty()) {
			widgets.icon.hide();
		} else {
			widgets.icon.set_from_icon_name(str->c_str(),Gtk::ICON_SIZE_DIALOG);
			widgets.icon.show();
		}

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
		set_title(object.get_label().c_str());
		set_sub_title(_("Initializing"));

#ifdef DEBUG
		action().set_text("action");
		step().set_text("step");
#else
		action().set_text("");
		step().set_text("");
#endif // DEBUG

		Gtk::Window::set_title(object.get_label());

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

