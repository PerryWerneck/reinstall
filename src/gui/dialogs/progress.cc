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
 #include <udjat/version.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>
 #include <sstream>
 #include <iomanip>

 using namespace Udjat;
 using namespace Gtk;
 using namespace std;

 static std::string format_size(double value);

 Dialog::Progress::Progress() {

	Gtk::Box &content_area = *get_content_area();

	set_decorated(false);

	set_default_size(500,-1);

	get_style_context()->add_class("dialog-progress");
	content_area.get_style_context()->add_class("dialog-contents");
	content_area.set_border_width(12);
	content_area.set_spacing(0);

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

	content_area.pack_start(widgets.header,false,false,6);

	widgets.progress.set_valign(ALIGN_CENTER);
	widgets.progress.get_style_context()->add_class("dialog-progress-bar");
	content_area.pack_start(widgets.progress,true,true,6);

	widgets.footer.get_style_context()->add_class("dialog-footer");
	widgets.footer.set_homogeneous(true);
	widgets.footer.pack_start(widgets.left,true,true,0);
	widgets.footer.pack_end(widgets.right,true,true,0);

	content_area.pack_end(widgets.footer,false,false,6);

	widgets.footer.set_valign(ALIGN_END);

	content_area.show_all();

	timer.source = Glib::TimeoutSource::create(100);

	timer.source->connect([this]{

		if(!is_visible()) {
			return true;
		}

		if(values.changed) {

			timer.idle = 0;
			values.changed = false;

			if(values.total > values.current && values.total > 1.0) {

				timer.idle = 0;
				widgets.progress.set_fraction(((gdouble) values.current) / ((gdouble) values.total));

				string fcurrent{format_size(values.current)};
				string ftotal{format_size(values.total)};

				widgets.right.set_text(
					Logger::Message{
						_("{} of {}"),
						fcurrent,
						ftotal
					}.c_str()
				);

			} else {

				widgets.right.set_text("");

			}

		} else if(timer.idle >= 100) {
			widgets.progress.pulse();
		} else {
			timer.idle++;
		}

		return true;
	});

	timer.source->attach(Glib::MainContext::get_default());

#ifdef DEBUG
	widgets.left.set_text("left");
	widgets.right.set_text("right");
#endif // DEBUG

 }

 Dialog::Progress::~Progress() {
 	debug("Progress dialog was deleted");
	timer.source->destroy();
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

 void Dialog::Progress::dismiss(int response_id) {

 	Glib::signal_idle().connect([this,response_id](){
		debug("Dismissing dialog with id ",response_id);
		response(response_id);
		return 0;
 	});

 }

 void Dialog::Progress::show() {

	debug("Asking for show progress");
 	Glib::signal_idle().connect([this](){
		debug("Showing progress dialog");
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

 void Dialog::Progress::set_url(const char *url) {

	timer.idle = 0;

	auto str = make_shared<string>(url);
	Glib::signal_idle().connect([this,str](){

		switch(url_mode) {
		case HIDE_URL:
			break;

		case SHOW_URL_ON_SUBTITLE:
			widgets.subtitle.set_text(str->c_str());
			widgets.progress.set_show_text(false);
			break;

		case SHOW_URL_ON_PROGRESS_BAR:
			widgets.progress.set_text(str->c_str());
			widgets.progress.set_show_text(true);
			break;

		}

		widgets.progress.set_fraction(0.0);
		widgets.right.set_text("");

		return 0;
	});

 }

 void Dialog::Progress::set_step(const char *step)  {

	auto str = make_shared<string>(step);

 	Glib::signal_idle().connect([this,str](){
		widgets.left.set_text(str->c_str());
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

 std::string format_size(double value) {

#if UDJAT_CORE_BUILD > 22123100

	return Udjat::String{}.set_byte(value,1);

#else

	static const struct {
		double value;
		const char *name;
	} sizes[] = {
		{ 1073741824.0, "Gb" },
		{    1048576.0, "Mb" },
		{       1024.0, "Kb" },
	};

	double unit_value = 1;
	const char * unit_name = "";

	for(size_t ix = 0; ix < N_ELEMENTS(sizes); ix++) {

		if(value > sizes[ix].value) {
			unit_value = sizes[ix].value;
			unit_name = sizes[ix].name;
			break;
		}

	}

	std::stringstream formatted;
	formatted << std::fixed << std::setprecision(1) << (value/unit_value) << " " << unit_name;
	return formatted.str();

#endif // UDJAT_CORE_BUILD

 }

 void Dialog::Progress::set(const Reinstall::Abstract::Object &object) {

	// object.set_dialog(*this);
	set_title(object.get_label().c_str());
	set_sub_title(_("Initializing"));

#ifdef DEBUG
	widgets.left.set_text("left");
	widgets.right.set_text("right");
#else
	widgets.left.set_text("");
	widgets.left.set_text("");
#endif // DEBUG

	Gtk::Window::set_title(object.get_label());

	timer.idle = -1;

 }

