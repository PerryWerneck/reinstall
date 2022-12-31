/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2022 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/defs.h>
 #include <gtkmm.h>
 #include <glibmm/i18n.h>
 #include <reinstall/dialogs/taskrunner.h>
 #include <private/dialogs.h>
 #include <udjat/tools/threadpool.h>
 #include <iostream>
 #include <udjat/tools/logger.h>

 using namespace std;
 using namespace Udjat;

 Dialog::TaskRunner::TaskRunner(Gtk::Window &parent, const char *message, bool markup)
	: Gtk::MessageDialog{parent,message,markup,Gtk::MESSAGE_INFO,Gtk::BUTTONS_NONE,true} {

	set_deletable(false);
	Gtk::Window::set_title(parent.get_title());

 }

 void Dialog::TaskRunner::set(const Reinstall::Action &action) {
	Gtk::Window::set_title(action.get_label().c_str());
 }

 void Dialog::TaskRunner::show() {
  	Glib::signal_idle().connect([this](){
		Gtk::Window::show();
		return 0;
 	});
 }

 int Dialog::TaskRunner::push(const std::function<int()> &callback, bool s) {

	int response = -1;
	auto mainloop = Glib::MainLoop::create();

	ButtonFactory("test",[]{
		cout << "Test button was clicked" << endl;
	});

	show_all();

	Udjat::ThreadPool::getInstance().push([this,&callback,&response,mainloop](){

		response = Reinstall::Dialog::TaskRunner::push(callback,false);

		Glib::signal_idle().connect([mainloop](){
			mainloop->quit();
			return 0;
		});

	});

	if(s) {
		show();
	}

	mainloop->run();

	return response;
 }

 void dismiss(int response_id);

 void Dialog::TaskRunner::set_title(const char *text, bool markup) {

 	auto str = make_shared<string>(text);
	Glib::signal_idle().connect([this,str,markup](){
		this->set_message(str->c_str(),markup);
		return 0;
	});

 }

 void Dialog::TaskRunner::set_sub_title(const char *text, bool markup) {

 	auto str = make_shared<string>(text);
	Glib::signal_idle().connect([this,str,markup](){
		this->set_secondary_text(str->c_str(),markup);
		return 0;
	});

 }

 std::shared_ptr<Reinstall::Dialog::Button> Dialog::TaskRunner::ButtonFactory(const char *label, const std::function<void()> &callback) {

	class Button : public Reinstall::Dialog::Button, public Gtk::Button {
	private:
		const std::function<void()> callback;

	public:
		Button(const char *label, const std::function<void()> c) : Gtk::Button(label), callback{c} {

			set_focus_on_click(false);
			show_all();

		}

		void enable(bool enabled) override {
			Glib::signal_idle().connect([this,enabled](){
				set_sensitive(enabled);
				return 0;
			});
		}

		void set_destructive() override {
			Glib::signal_idle().connect([this](){
				get_style_context()->add_class("destructive-action");
				return 0;
			});
		}

		void set_suggested() override {
			Glib::signal_idle().connect([this](){
				get_style_context()->add_class("suggested-action");
				return 0;
			});
		}

		void activate() override {

			Glib::signal_idle().connect([this](){
				try {

					callback();

				} catch(const std::exception &e) {

					cerr << e.what() << endl;

				} catch(...) {

					cerr << "gtk\tUnexpected error activating taskrunner button" << endl;

				}
				return 0;
			});

		}

		void on_clicked() {
			activate();
		}

	};

	debug("-----------------------------------------------------------------");

	std::shared_ptr<Button> button = make_shared<Button>(label,callback);

	button->show_all();
	add_action_widget(*button,0);

	return button;

 }
