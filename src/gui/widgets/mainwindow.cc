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
 #include <udjat/version.h>
 #include <private/mainwindow.h>
 #include <private/dialogs.h>
 #include <reinstall/object.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/application.h>
 #include <reinstall/controller.h>
 #include <reinstall/tools.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/file.h>
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

 /*
#ifdef DEBUG
	{
		Dialog::Progress dialog;
		dialog.Gtk::Window::set_title(get_title());
		dialog.set_parent(*this);
		dialog.set_title(_("Getting configuration"));
		dialog.set_url("http://www.google.com");
		dialog.set_icon_name("dialog-information");
		dialog.footer(true);
		dialog.set_decorated(true);
		dialog.set_deletable(true);
		dialog.show();
		dialog.run();
	}
#endif // DEBUG
*/

	// Load options
	{
		Dialog::Progress dialog;
		dialog.Gtk::Window::set_title(get_title());
		dialog.set_parent(*this);
		dialog.set_title(_("Getting configuration"));
		dialog.set_sub_title(_("Loading option list"));
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

 static bool check_file(const Gtk::Entry &entry, bool save) {

		Udjat::File::Path file{entry.get_text()};

		if(file.empty() || file.dir()) {
			return false;
		}

#if UDJAT_CORE_BUILD >= 22122800
		if(save || file.regular()) {
			return true;
		}
		return false;
#else
		return true;
#endif
}

 std::string MainWindow::FilenameFactory(const char *title, const char *label_text, const char *apply, const char *filename, bool save) {

	Gtk::Entry entry;
	Gtk::Box box{Gtk::ORIENTATION_HORIZONTAL,6};
	Gtk::Label label{ label_text, Gtk::ALIGN_END };
	Gtk::Dialog dialog{title,true};

	dialog.set_default_size(600, -1);

	dialog.set_modal(true);
	dialog.set_transient_for(*this);

	entry.set_hexpand(true);
	entry.set_activates_default(true);
	entry.set_icon_from_icon_name(save ? "document-save-as" : "document-open",Gtk::ENTRY_ICON_SECONDARY);
	entry.set_icon_activatable(true);

	if(filename && *filename) {

		Udjat::String path{filename};
		path.expand(); // resolve ${variable}

#ifdef _WIN32
		if(path[0] == '\\') {
			entry.set_text(path.c_str());
		} else {
			entry.set_text(std::string{Glib::get_user_special_dir(Glib::USER_DIRECTORY_DOCUMENTS)} + "\\" + path);
		}
#else
		if(*filename == '/') {
			entry.set_text(path.c_str());
		} else {
			entry.set_text(std::string{Glib::get_user_special_dir(Glib::USER_DIRECTORY_DOCUMENTS)} + "/" + path);
		}
#endif // _WIN32

	}

	entry.signal_icon_press().connect([&dialog,&entry,save,title](Gtk::EntryIconPosition, const GdkEventButton *) {

		Gtk::FileChooserDialog filechooser{
			dialog,
			title,
			(save ? Gtk::FILE_CHOOSER_ACTION_SAVE : Gtk::FILE_CHOOSER_ACTION_OPEN)
		};

		filechooser.set_filename(entry.get_text());
		if(filechooser.run() == Gtk::RESPONSE_ACCEPT) {
			entry.set_text(filechooser.get_filename());
		}

    });


	{
		Gtk::Box &carea = *dialog.get_content_area();
		carea.set_border_width(12);
		carea.set_spacing(6);
		carea.add(box);
	}

	{
		box.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
		box.add(label);
		box.add(entry);
	}

	dialog.add_button(_("_Cancel"),Gtk::RESPONSE_CANCEL);

	Gtk::Button &apply_button = *dialog.add_button(apply, Gtk::RESPONSE_APPLY);

	dialog.set_default(apply_button);
	apply_button.set_sensitive(check_file(entry,save));

	entry.signal_changed().connect([&entry,save,&apply_button]() {
		apply_button.set_sensitive(check_file(entry,save));
	});

	dialog.show_all();
	if(dialog.run() == Gtk::RESPONSE_APPLY) {
		return entry.get_text();
	}

	return "";
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

	//
	// Interact with user.
	//
	try {

		if(!action->interact()) {
			buttons.apply.set_sensitive(true);
			buttons.cancel.set_sensitive(true);
			layout.view.set_sensitive(true);
			return;
		}

	} catch(const std::exception &e) {

		Gtk::MessageDialog dialog_fail{
			*this,
			_("First step has failed"),
			false,
			Gtk::MESSAGE_ERROR,
			Gtk::BUTTONS_OK,
			true
		};

		dialog_fail.set_default_size(500, -1);
		dialog_fail.set_title(action->get_label());
		dialog_fail.set_secondary_text(e.what());
		dialog_fail.show();
		dialog_fail.run();

		return;
	}

	//
	// Ask for confirmation.
	//
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

	if(response == Gtk::RESPONSE_YES) {
		//
		// Build and burn image.
		//
		std::string error_message;
		Dialog::Progress dialog;

		dialog.set_parent(*this);
		dialog.set_decorated(false);
		dialog.set_deletable(false);
		dialog.set(*action->get_button());
		dialog.show();

		Udjat::ThreadPool::getInstance().push([&dialog,action,&error_message](){

			try {

				action->prepare();
				action->burn();

			} catch(const std::exception &e) {

				error_message = e.what();
				cerr << e.what() << endl;

			}

			dialog.dismiss();

		});

		dialog.run();
		dialog.hide(); // Just hide to wait for all enqueued state changes to run.

		{
			std::shared_ptr<Gtk::MessageDialog> popup;

			if(error_message.empty()) {

				if(action->success()) {

					// Customized success dialog.

					cout << "MainWindow\tShowing action 'success' dialog" << endl;
					popup = make_shared<Dialog::Popup>(
						*this,
						*action->get_button(),
						action->success(),
						Gtk::MESSAGE_INFO,
						Gtk::BUTTONS_OK
					);

				} else {

					// Standard success dialog.

					cout << "MainWindow\tShowing default 'success' dialog" << endl;
					popup = make_shared<Gtk::MessageDialog>(
						*this,
						_("Action completed"),
						false,
						Gtk::MESSAGE_INFO,
						Gtk::BUTTONS_OK,
						true
					);

				}

				// Add extra buttons.
				if(action->reboot()) {

					// Close button is the suggested action.
					auto close = popup->get_widget_for_response(Gtk::RESPONSE_CLOSE);
					close->get_style_context()->add_class("suggested-action");
					popup->set_default_response(Gtk::RESPONSE_CLOSE);

					// Reboot button is destructive.
					auto reboot = popup->add_button(_("Reboot"),Gtk::RESPONSE_APPLY);
					reboot->get_style_context()->add_class("destructive-action");

				}

			} else if(action->failed()) {

				// Customized error dialog.

				popup = make_shared<Dialog::Popup>(
					*this,
					*action->get_button(),
					action->failed(),
					Gtk::MESSAGE_ERROR,
					Gtk::BUTTONS_OK
				);

				if(!action->failed().has_secondary()) {
					popup->set_secondary_text(error_message);
				}

			} else {

				// Standard error dialog.

				popup = make_shared<Gtk::MessageDialog>(
					*this,
					_("Action has failed"),
					false,
					Gtk::MESSAGE_ERROR,
					Gtk::BUTTONS_OK,
					true
				);

				popup->set_secondary_text(error_message);

			}

			{
				auto cancel = popup->add_button(_("Quit application"),Gtk::RESPONSE_CANCEL);
				if(!action->reboot() && error_message.empty()) {
					cancel->get_style_context()->add_class("suggested-action");
					popup->set_default_response(Gtk::RESPONSE_CANCEL);
				}
			}
	
			popup->set_title(action->get_label());
			popup->set_default_size(500, -1);
			popup->show();
			switch(popup->run()) {
			case Gtk::RESPONSE_APPLY:
				cout << "MainWindow\tRebooting by user request" << endl;
				Reinstall::reboot();
				Gtk::Application::get_default()->quit();
				break;

			case Gtk::RESPONSE_CANCEL:
				Gtk::Application::get_default()->quit();
				break;
			}


		}
	}

	buttons.apply.set_sensitive(true);
	buttons.cancel.set_sensitive(true);
	layout.view.set_sensitive(true);

 }
