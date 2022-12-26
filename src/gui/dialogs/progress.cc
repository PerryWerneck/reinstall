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

	content_area.get_style_context()->add_class("dialog-contents");
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

 int Dialog::Progress::on_worker(std::shared_ptr<Worker> worker) noexcept {
	worker->work(*this);
	return 0;
 }

 void Dialog::Progress::enqueue(std::shared_ptr<Worker> worker) noexcept {
	Glib::signal_idle().connect(sigc::bind<1>( sigc::mem_fun(this,&::Dialog::Progress::on_worker),worker) );
 }

 void Dialog::Progress::show() {

	class Show : public Worker {
		void work(Progress &dialog) const noexcept override {
			dialog.Gtk::Dialog::show();
		}
	};

	enqueue(make_shared<Show>());

 }

 void Dialog::Progress::hide() {

	class Hide : public Worker {
	public:
		void work(Progress &dialog) const noexcept override {
			dialog.Gtk::Dialog::hide();
		}
	};

	enqueue(make_shared<Hide>());

 }

 void Dialog::Progress::set(const char *message)  {

	class SetMessage : public Worker, public string {
	public:
		SetMessage(const char *msg) : string{msg} {
		}

		void work(Progress &dialog) const noexcept override {
			dialog.widgets.title.set_text(c_str());
		}

	};

	enqueue(make_shared<SetMessage>(message));

 }

 void Dialog::Progress::count(size_t count, size_t total)  {

	class SetCount : public Worker {
	public:
		size_t count, total;

		SetCount(size_t c, size_t t) : count{c}, total{t} {
		}

		void work(Progress &dialog) const noexcept override {
		}

	};

	enqueue(make_shared<SetCount>(count,total));

 }

 void Dialog::Progress::update(double current, double total)  {

	class Update : public Worker {
	public:
		float current, total;

		Update(float c, float t) : current{c}, total{t} {
		}

		void work(Progress &dialog) const noexcept override {

			if(total > current && total > 1) {
				dialog.timer.idle = 0;
				gdouble fraction = ((gdouble) current) / ((gdouble) total);
				dialog.widgets.progress.set_fraction(fraction);
			} else {
				dialog.widgets.step.set_text("");
			}

		}

	};

	enqueue(make_shared<Update>(current,total));

 }

 void Dialog::Progress::set(const Reinstall::Object &object) {

	class ObjectSet : public Worker {
	public:
		const Reinstall::Object &object;

		ObjectSet(const Reinstall::Object &o) : object{o} {
		}

		void work(Progress &dialog) const noexcept override {

			object.set_dialog(dialog);
			dialog.sub_title().set_text(_("Initializing"));
			dialog.action().set_text("");
			dialog.message().set_text("");
			dialog.step().set_text("");

			dialog.timer.idle = -1;

			if(object.icon && *object.icon) {

				// https://developer-old.gnome.org/gtkmm/stable/classGtk_1_1Image.html
				dialog.icon().set_from_icon_name(object.icon,Gtk::ICON_SIZE_DND);
				dialog.icon().show();

			} else {

				dialog.icon().hide();

			}
		}

	};

	enqueue(make_shared<ObjectSet>(object));

 }

