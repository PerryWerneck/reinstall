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
  * @brief Implement task with progress dialog.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <gtkmm.h>
 #include <glibmm/i18n.h>
 #include <private/mainwindow.h>
 #include <udjat/ui/dialogs/progress.h>
 #include <udjat/tools/threadpool.h>
 #include <memory>

 using namespace Udjat;
 using namespace Gtk;
 using namespace std;

 int MainWindow::run(const Udjat::Dialog &dialog, const std::function<int(Udjat::Dialog::Progress &progress)> &task) {

	class ProgressDialog : public Gtk::Dialog, public Udjat::Dialog::Progress {
	private:
		class Label : public Gtk::Label {
		public:
			Label(const char *text, Gtk::Align halign = Gtk::ALIGN_START, Gtk::Align valign=Gtk::ALIGN_CENTER) : Gtk::Label(text,halign,valign) {
			}

			inline Label & operator = (const char *value) {
				set_text(value);
				return *this;
			}

		};

		struct {
			Glib::RefPtr<Glib::TimeoutSource> source;
			unsigned int idle = (unsigned int) -1;
		} timer;

		struct {
			Label title{ "" };
			Label message{ "" };
			Label left{ "", Gtk::ALIGN_START };
			Label right{ "", Gtk::ALIGN_END };

			Gtk::Grid header;
			Gtk::Image icon;
			Gtk::ProgressBar progress;
			Gtk::Box footer{Gtk::ORIENTATION_HORIZONTAL};

		} widgets;

		struct {
			std::string title;
			std::string message;
			std::string url;
		} text;

		struct {
			bool changed = false;
			double current = 0;
			double total = 0;
		} values;

	public:
		ProgressDialog(Gtk::Window &parent, const Udjat::Dialog &dialog) {

			set_modal(true);
			set_transient_for(parent);

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

			widgets.message.get_style_context()->add_class("dialog-message");
			widgets.message.set_hexpand(true);
			widgets.message.set_vexpand(false);
			widgets.message.set_valign(ALIGN_START);
			widgets.message.set_line_wrap(false);
			widgets.message.set_ellipsize(Pango::ELLIPSIZE_START);
			widgets.header.attach(widgets.message,0,1,1,1);

			widgets.icon.get_style_context()->add_class("dialog-icon");
			widgets.icon.set_hexpand(false);
			widgets.icon.set_vexpand(false);
			widgets.header.attach(widgets.icon,1,0,2,2);

			content_area.pack_start(widgets.header,false,false,6);

			widgets.progress.set_valign(ALIGN_CENTER);
			widgets.progress.get_style_context()->add_class("dialog-progress-bar");
			widgets.progress.set_ellipsize(Pango::ELLIPSIZE_START);
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

		virtual ~ProgressDialog() {
			timer.source->destroy();
		}

		void title(const char *title) override {
			text.title = title;
			Glib::signal_idle().connect([this](){
				widgets.title.set_text(text.title.c_str());
				return 0;
			});
		}

		void message(const char *message) override {
			text.title = message;
			Glib::signal_idle().connect([this](){
				widgets.message.set_text(text.message.c_str());
				return 0;
			});
		}

		std::string message() const override {
			return text.message;
		}

		void icon(const char *icon_name) override {
		}

		void url(const char *url) override {
			text.url = url;
			Glib::signal_idle().connect([this](){
				widgets.progress.set_text(text.url.c_str());
				widgets.progress.set_show_text(!text.url.empty());
				return 0;
			});
		}

		void pulse() override {
			timer.idle = 1000;
		}

		void progress(double current, double total) override {
			values.current = current;
			values.total = total;
			values.changed = true;
		}

		void count(size_t current, size_t total) override {

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

	};

	ProgressDialog progress{*this,dialog};
	progress.show_all();

	std::string error_message;
	Udjat::ThreadPool::getInstance().push([&progress,task,&error_message](){

		int rc = -1;

		try {

			rc = task(progress);

		} catch(const std::exception &e) {
			rc = -1;
			error_message = e.what();
			Logger::String{error_message}.error("ui-task");
		} catch(...) {
			rc = -1;
			error_message = _("Unexpected error on background task");
			Logger::String{error_message}.error("ui-task");
		}

		Glib::signal_idle().connect([&progress,rc](){
			progress.response(rc);
			return 0;
		});

	});

	int rc = progress.run();

	if(rc == -1 && !error_message.empty()) {
		throw runtime_error(error_message);
	}

	return rc;

 }

