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
  * @brief Brief description of this source.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/ui/dialog.h>
 #include <udjat/ui/gtk/progress.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/threadpool.h>
 #include <memory>

 using namespace std;

 namespace Udjat {

	Gtk::Progress::Progress(::Gtk::Window &parent, const Udjat::Dialog &dialog) {

		text.title = dialog.message;
		widgets.title.set_text(text.title.c_str());

		text.message = dialog.secondary;
		widgets.message.set_text(text.message.c_str());

		set_modal(true);
		set_transient_for(parent);

		if(dialog.title && *dialog.title) {
			set_title(dialog.title);
		} else {
			set_title(parent.get_title());
		}

		auto &content_area = *get_content_area();

		set_decorated(false);
		set_default_size(500,-1);

		get_style_context()->add_class("dialog-progress");
		content_area.get_style_context()->add_class("dialog-contents");
		content_area.set_border_width(12);
		content_area.set_spacing(0);

		content_area.set_homogeneous(false);
		content_area.set_orientation(::Gtk::ORIENTATION_VERTICAL);

		widgets.title.get_style_context()->add_class("dialog-title");
		widgets.title.set_hexpand(true);
		widgets.title.set_vexpand(false);
		widgets.title.set_valign(::Gtk::ALIGN_START);
		widgets.title.set_line_wrap(false);
		widgets.title.set_ellipsize(Pango::ELLIPSIZE_START);
		widgets.header.attach(widgets.title,0,0,1,1);

		widgets.message.get_style_context()->add_class("dialog-message");
		widgets.message.set_hexpand(true);
		widgets.message.set_vexpand(false);
		widgets.message.set_valign(::Gtk::ALIGN_START);
		widgets.message.set_line_wrap(false);
		widgets.message.set_ellipsize(Pango::ELLIPSIZE_START);
		widgets.header.attach(widgets.message,0,1,1,1);

		widgets.icon.get_style_context()->add_class("dialog-icon");
		widgets.icon.set_hexpand(false);
		widgets.icon.set_vexpand(false);
		widgets.header.attach(widgets.icon,1,0,2,2);

		content_area.pack_start(widgets.header,false,false,6);

		widgets.progress.set_valign(::Gtk::ALIGN_CENTER);
		widgets.progress.get_style_context()->add_class("dialog-progress-bar");
		widgets.progress.set_ellipsize(Pango::ELLIPSIZE_START);
		content_area.pack_start(widgets.progress,true,true,6);

		widgets.footer.get_style_context()->add_class("dialog-footer");
		widgets.footer.set_homogeneous(true);
		widgets.footer.pack_start(widgets.left,true,true,0);
		widgets.footer.pack_end(widgets.right,true,true,0);

		// widgets.left.set_max_width_chars(30);
		// widgets.left.set_lines(1);
		widgets.left.set_single_line_mode(true);
		widgets.left.set_ellipsize(Pango::ELLIPSIZE_END);

		// widgets.right.set_max_width_chars(30);
		// widgets.right.set_lines(1);
		widgets.right.set_single_line_mode(true);
		widgets.right.set_ellipsize(Pango::ELLIPSIZE_START);

		content_area.pack_end(widgets.footer,false,false,6);

		widgets.footer.set_valign(::Gtk::ALIGN_END);

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

					widgets.right.set_text(
						Logger::Message{
							_("{} of {}"),
							String{}.set_byte(values.current),
							String{}.set_byte(values.total)
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

	Gtk::Progress::~Progress() {
		debug("Dialog was destroyed");
		timer.source->destroy();
	}

	void Gtk::Progress::title(const char *title)  {
		text.title = title;
		Glib::signal_idle().connect([this](){
			debug("title='",text.title.c_str(),"'");
			widgets.title.set_text(text.title.c_str());
			return 0;
		});
	}

	void Gtk::Progress::message(const char *message)  {
		text.message = message;
		Glib::signal_idle().connect([this](){
			debug("message='",text.message.c_str(),"'");
			widgets.message.set_text(text.message.c_str());
			return 0;
		});
	}

	std::string Gtk::Progress::message() const  {
		return text.message;
	}

	void Gtk::Progress::icon(const char *icon_name)  {
	}

	void Gtk::Progress::url(const char *url)  {
		text.url = url;
		Glib::signal_idle().connect([this](){
			widgets.progress.set_text(text.url.c_str());
			widgets.progress.set_show_text(!text.url.empty());
			return 0;
		});
	}

	void Gtk::Progress::pulse()  {
		timer.idle = 1000;
	}

	void Gtk::Progress::progress(double current, double total)  {
		values.current = current;
		values.total = total;
		values.changed = true;
	}

	void Gtk::Progress::count(size_t current, size_t total)  {

		if(total) {
			auto str = make_shared<Logger::Message>(_("{} of {}"),current,total);
			Glib::signal_idle().connect([this,str](){
				widgets.left.set_text(str->c_str());
				return 0;
			});
		} else {
			Glib::signal_idle().connect([this](){
				widgets.left.set_text("");
				return 0;
			});
		}

	}

	int Gtk::Progress::run(const std::function<int(Udjat::Dialog::Progress &progress)> &task) {

		std::string error_message;

		// We cant start thread before the widget initialization is complete.
		auto connection = signal_show().connect([this,task,&error_message]{

			// Widget is showing, start background thread.
			auto application = ::Gtk::Application::get_default();
			application->mark_busy();
			Udjat::ThreadPool::getInstance().push([this,task,&error_message](){

				usleep(100);

				int rc = -1;

				try {

					rc = task(*this);

				} catch(const std::exception &e) {
					rc = -1;
					error_message = e.what();
					Logger::String{error_message}.error("ui-task");
				} catch(...) {
					rc = -1;
					error_message = _("Unexpected error on background task");
					Logger::String{error_message}.error("ui-task");
				}

				debug("Background task complete");
				Glib::signal_idle().connect([this,rc](){
					Logger::String{"Background task complete with rc=",rc}.trace("progress");
					response(rc);
					return 0;
				});

			});
			application->unmark_busy();

		});

		pulse();

		::Gtk::Dialog::show_all();
		int rc = ::Gtk::Dialog::run();
		::Gtk::Dialog::hide();

		connection.disconnect();

		if(rc == -1 && !error_message.empty()) {
			throw runtime_error(error_message);
		}

		return rc;

	}


 }
