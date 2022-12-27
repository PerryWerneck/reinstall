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
 #include <private/mainwindow.h>
 #include <private/dialogs.h>
 #include <reinstall/object.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/application.h>
 #include <reinstall/controller.h>
 #include <udjat/tools/logger.h>
 #include <private/widgets.h>
 #include <udjat/module.h>
 #include <iostream>

 using namespace std;
 using namespace Udjat;

 MainWindow::MainWindow() {

 	{
		auto css = Gtk::CssProvider::create();
#ifdef DEBUG
		css->load_from_path("./stylesheet.css");
#else
		css->load_from_path(Application::DataFile("stylesheet.css").c_str());
#endif // DEBUG
		get_style_context()->add_provider_for_screen(Gdk::Screen::get_default(), css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
 	}

	set_title(_("System reinstallattion"));
	set_default_size(600, 400);

	// Left box
	{
		Gtk::Box * box = new Gtk::Box(Gtk::ORIENTATION_VERTICAL);
		box->get_style_context()->add_class("sidebar");

		box->set_hexpand(false);
		box->set_vexpand(true);
		box->add(logo);

		layout.hbox.add(*box);
	}

	// https://developer-old.gnome.org/gtkmm-tutorial/stable/sec-clipboard-examples.html.en
	// http://transit.iut2.upmf-grenoble.fr/doc/gtkmm-3.0/tutorial/html/sec-helloworld.html

	layout.vbox.set_hexpand(true);
	layout.vbox.set_vexpand(true);

	// A wide variety of style classes may be applied to labels, such as .title, .subtitle, .dim-label, etc
	layout.title.get_style_context()->add_class("main-title");
	layout.vbox.pack_start(layout.title,false,false,0);

	layout.view.get_style_context()->add_class("main-view");

	layout.swindow.set_hexpand(true);
	layout.swindow.set_vexpand(true);
	layout.swindow.add(layout.view);
	layout.vbox.add(layout.swindow);

	buttons.apply.set_sensitive(false);
	buttons.apply.signal_clicked().connect([&]() {

		apply();

    });

	buttons.cancel.signal_clicked().connect([&]() {
		close();
    });

	layout.bbox.set_layout(Gtk::BUTTONBOX_END);
	layout.bbox.add(buttons.cancel);
	layout.bbox.add(buttons.apply);
	layout.bbox.set_hexpand(true);
	layout.vbox.add(layout.bbox);

	layout.bbox.set_spacing(6);

	layout.hbox.set_hexpand(true);
	layout.hbox.set_vexpand(true);
	layout.hbox.add(layout.vbox);

	layout.hbox.get_style_context()->add_class("main-window");
	add(layout.hbox);
	layout.hbox.show_all();

 }

 MainWindow::~MainWindow() {
 	Reinstall::Controller::getInstance().clear();
	Udjat::Application::finalize();
 }

 void MainWindow::on_show() {

#ifdef DEBUG
	{
		Dialog::Progress dialog;
		dialog.Gtk::Window::set_title(get_title());
		dialog.set_parent(*this);
		dialog.set_title(_("Getting configuration"));
		dialog.set_sub_title("http://www.google.com");
		dialog.set_icon_name("dialog-information");
		dialog.footer(true);
		dialog.set_decorated(true);
		dialog.set_deletable(true);
		dialog.show();
		dialog.run();
	}
#endif // DEBUG

	// Load options
	{
		Dialog::Progress dialog;
		dialog.Gtk::Window::set_title(get_title());
		dialog.set_parent(*this);
		dialog.set_title(_("Getting configuration"));
		dialog.set_icon_name("");
		dialog.footer(false);
		dialog.set_decorated(true);
		dialog.set_deletable(false);
		dialog.show();

		Udjat::ThreadPool::getInstance().push([&dialog](){

			// First get controller to construct the factories.
			Reinstall::Controller::getInstance();

			// Load application modules
#ifdef DEBUG
			Udjat::Module::load(".bin/Debug/modules");
#endif // DEBUG

			// Load image definitions.
			Udjat::Application::setup("./xml.d",true);

			// And dismiss dialog.
			dialog.dismiss();

		});

		dialog.run();
	}

	buttons.apply.set_sensitive(Reinstall::Action::get_selected() != nullptr);

	layout.view.show_all();

	Gtk::Window::on_show();

 }

 std::shared_ptr<Reinstall::Abstract::Object> MainWindow::ActionFactory(const pugi::xml_node &node) {
	return make_shared<::Widget::Action>(node);
 }

 std::shared_ptr<Reinstall::Abstract::Group> MainWindow::GroupFactory(const pugi::xml_node &node) {
 	auto group = make_shared<::Widget::Group>(node);

 	Glib::signal_idle().connect([this,group](){
		layout.view.pack_start(*group,true,true,0);
		return 0;
 	});

	return group;
 }

 void MainWindow::apply() {

	Reinstall::Action *action = Reinstall::Action::get_selected();

	if(!action) {
		cerr << "Apply with no selected action" << endl;
		return;
	}

 	g_message("Apply '%s'",std::to_string(*action).c_str());
	buttons.apply.set_sensitive(false);
	buttons.cancel.set_sensitive(false);
	layout.view.set_sensitive(false);

	Gtk::ResponseType response = Gtk::RESPONSE_YES;
	if(action->confirmation()) {
		response = (Gtk::ResponseType) Dialog::Popup{
						*this,
						*action->get_button(),
						action->confirmation(),
						Gtk::MESSAGE_QUESTION,
						Gtk::BUTTONS_YES_NO
					}.run();
	}

	// Execute action
	if(response == Gtk::RESPONSE_YES) {

		std::string error_message;
		Dialog::Progress dialog;

		dialog.set_parent(*this);
		dialog.set_decorated(false);
		dialog.set_deletable(false);
		dialog.set(*action->get_button());
		dialog.show();

		Udjat::ThreadPool::getInstance().push([&dialog,action,&error_message,this](){

			try {

				action->prepare();

			} catch(const std::exception &e) {

				error_message = e.what();
				cerr << e.what() << endl;

			}

			dialog.dismiss();

		});

		dialog.run();
		dialog.hide(); // Just hide to wait for all enqueued state changes to run.

		if(!error_message.empty()) {

			if(action->failed()) {

				Dialog::Popup dialog_fail{
					*this,
					*action->get_button(),
					action->failed(),
					Gtk::MESSAGE_ERROR,
					Gtk::BUTTONS_CLOSE
				};

				if(!action->failed().has_secondary()) {
					dialog_fail.set_secondary_text(error_message);
				}
				dialog_fail.run();

			} else {

				Gtk::MessageDialog dialog_fail{
					*this, // Gtk::Window& parent,
					_("Action has failed"), // const Glib::ustring& message,
					false,	// bool use_markup = false,
					Gtk::MESSAGE_ERROR, // MessageType type =
					Gtk::BUTTONS_CLOSE, // ButtonsType buttons = BUTTONS_OK,
					true
				};

				dialog_fail.set_default_size(500, -1);
				dialog_fail.set_title(action->get_label());
				dialog_fail.set_secondary_text(error_message);
				dialog_fail.show();
				dialog_fail.run();

			}

		} else if(action->success()) {

			Dialog::Popup{
				*this,
				*action->get_button(),
				action->success(),
				Gtk::MESSAGE_INFO,
				Gtk::BUTTONS_OK
			}.run();

		}

	}

	buttons.apply.set_sensitive(true);
	buttons.cancel.set_sensitive(true);
	layout.view.set_sensitive(true);

 }
