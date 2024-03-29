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

 #include <config.h>
 #include <udjat/version.h>
 #include <private/mainwindow.h>
 #include <private/dialogs.h>
 #include <reinstall/object.h>
 #include <reinstall/builder.h>
 #include <reinstall/writer.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/application.h>
 #include <reinstall/controller.h>
 #include <reinstall/tools.h>
 #include <udjat/tools/logger.h>
 #include <private/widgets.h>
 #include <udjat/tools/configuration.h>
 #include <iostream>
 #include <udjat/tools/object.h>

 using namespace std;
 using namespace Udjat;

 static const Udjat::ModuleInfo moduleinfo{PACKAGE_NAME " Main window"};

 MainWindow::MainWindow() : Udjat::Factory("MainWindow",moduleinfo) {

 	{
		auto css = Gtk::CssProvider::create();
#ifdef DEBUG
		css->load_from_path("./stylesheet.css");
#else
		css->load_from_path(Application::DataFile("stylesheet.css").c_str());
#endif // DEBUG
		get_style_context()->add_provider_for_screen(Gdk::Screen::get_default(), css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
 	}

 	set_title(Config::Value<string>("MainWindow","title",_("System reinstallation")));
	set_default_size(800, 600);

	// MainWindow logo
#ifdef DEBUG
	set_icon(Config::Value<string>{"MainWindow","icon","./icon.svg"}.c_str());
#else
	set_icon(Config::Value<string>{"MainWindow","icon",PRODUCT_ID "." PACKAGE_NAME}.c_str());
#endif // DEBUG


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

	layout.view.set_valign(Gtk::ALIGN_START);
	layout.view.set_halign(Gtk::ALIGN_FILL);
	layout.view.get_style_context()->add_class("main-view");

	layout.swindow.set_hexpand(true);
	layout.swindow.set_vexpand(true);
	layout.swindow.add(layout.view);
	layout.vbox.add(layout.swindow);

	buttons.apply.set_sensitive(false);
	buttons.apply.signal_clicked().connect([&]() {

		buttons.apply.set_sensitive(false);
		buttons.cancel.set_sensitive(false);
		layout.view.set_sensitive(false);
		apply();
		buttons.apply.set_sensitive(true);
		buttons.cancel.set_sensitive(true);
		layout.view.set_sensitive(true);

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
 	Application::finalize();
 }

 void MainWindow::on_show() {

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

			int response_id = GTK_RESPONSE_OK;

			try {

				// Load xml.d, initialize libudjat.
				Reinstall::Controller::getInstance().setup();

			} catch(const std::exception &e) {

				cerr << e.what() << endl;
				response_id = GTK_RESPONSE_CANCEL;

			} catch(...) {

				cerr << "Unexpected error loading configuration" << endl;
				response_id = GTK_RESPONSE_CANCEL;
			}

			// And dismiss dialog.
			dialog.dismiss(response_id);

		});

		if(dialog.run() != GTK_RESPONSE_OK) {
			Gtk::MessageDialog dialog_fail{
				*this,
				_("Initialization has failed"),
				false,
				Gtk::MESSAGE_ERROR,
				Gtk::BUTTONS_OK,
				true
			};

			dialog_fail.set_default_size(500, -1);
			dialog_fail.set_title(_("Error"));
			dialog_fail.set_secondary_text(_("The initialization procedure has failed, the application cant continue"));
			dialog_fail.show();
			dialog_fail.run();
			Gtk::Application::get_default()->quit();
			return;
		}
	}

	// TODO: Check for disabled actions.

	buttons.apply.set_sensitive( &Reinstall::Action::get_selected() != nullptr);

	layout.view.show_all();

	Gtk::Window::on_show();

 }

 std::shared_ptr<Reinstall::Abstract::Object> MainWindow::ActionFactory(const pugi::xml_node &node, const char *icon_name) {
	return make_shared<::Widget::Action>(node,icon_name);
 }

 std::shared_ptr<Reinstall::Dialog::TaskRunner> MainWindow::TaskRunnerFactory(const char *message, bool markup) {
	return make_shared<Dialog::TaskRunner>(*this,message,markup);
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

 void MainWindow::show(Dialog::Progress &dialog) {

	dialog.set_parent(*this);
	dialog.set_decorated(false);
	dialog.set_deletable(false);

	Reinstall::Action &action = Reinstall::Action::get_selected();

	dialog.set(*action.get_button());
	dialog.set_icon_name(action.get_icon_name());
	dialog.show();

 }

 void MainWindow::failed(const char *message) {

	std::shared_ptr<string> str = make_shared<string>(message);

	cerr << "MainWindow\tOperation failed: " << str->c_str() << endl;

 	Glib::signal_idle().connect([this,str](){

		Reinstall::Action &action = Reinstall::Action::get_selected();
		std::shared_ptr<Gtk::MessageDialog> popup;

		if(action.failed()) {

			// Customized error dialog.
			cout << "MainWindow\tShowing customized error popup for '" << str->c_str() << "'" << endl;

			popup = make_shared<Dialog::Popup>(
				*this,
				*action.get_button(),
				action.failed(),
				Gtk::MESSAGE_ERROR,
				Gtk::BUTTONS_OK
			);

			if(!action.failed().has_secondary()) {
				popup->set_secondary_text(str->c_str());
			}

		} else {

			// Standard error dialog.
			cout << "MainWindow\tShowing standard error popup for '" << str->c_str() << "'" << endl;

			popup = make_shared<Gtk::MessageDialog>(
				*this,
				_("Action has failed"),
				false,
				Gtk::MESSAGE_ERROR,
				Gtk::BUTTONS_OK,
				true
			);

			popup->set_secondary_text(str->c_str());

		}

		if(action.quit()) {
			Widget *cancel = popup->add_button(Config::Value<string>{"buttons","quit",_("Quit application")},Gtk::RESPONSE_CANCEL);
			cancel->get_style_context()->add_class("suggested-action");
			popup->set_default_response(Gtk::RESPONSE_CANCEL);
		}

		popup->set_title(action.get_label());
		popup->set_default_size(500, -1);
		popup->present();

		debug("Running popup");
		if(popup->run() == Gtk::RESPONSE_CANCEL) {
			cout << "MainWindow\tUser selected 'cancel' on error popup" << endl;
			Gtk::Application::get_default()->quit();
		} else {
			cout << "MainWindow\tUser selected 'Ok' on error popup" << endl;
		}

		return 0;
 	});

 }

 void MainWindow::set_icon(const char *icon) {

	if(access(icon, R_OK)) {

		// File not found, try icon name.
		debug("icon-name=",icon);
		set_icon_name(icon);
		set_default_icon_name(icon);

	} else {

		// File exists, use it.
		debug("icon-file=",icon);
		if(!set_icon_from_file(icon)) {
			cerr << "MainWindow\tUnable to set icon from '" << icon << "'" << endl;
		}

	}

 }

 void MainWindow::set_logo(const char *name) {
 	logo.set(name);
 }

 bool MainWindow::generic(const pugi::xml_node &node) {

	static const struct {
		const char *name;
		const std::function<void(MainWindow &hwnd, const char *value)> call;
	} attributes[] = {
		{
			"title",
			[](MainWindow &hwnd, const char *value){
				hwnd.set_title(value);
			}
		},
		{
			"sub-title",
			[](MainWindow &hwnd, const char *value){
				hwnd.layout.title.set_text(value);
			}
		},
		{
			"logo",
			[](MainWindow &hwnd, const char *value){
				hwnd.set_logo(value);
			}
		},
		{
			"icon",
			[](MainWindow &hwnd, const char *value){
				hwnd.set_icon(value);
			}
		},
		{
			"cancel",
			[](MainWindow &hwnd, const char *value){
				hwnd.buttons.cancel.set_label(value);
			}
		},
		{
			"apply",
			[](MainWindow &hwnd, const char *value){
				hwnd.buttons.apply.set_label(value);
			}
		}
	};

	for(auto &attr : attributes) {

		auto &attribute = Udjat::Object::getAttribute(node, attr.name);
		if(attribute) {
			attr.call(*this,attribute.as_string());
		}

	}

	return true;
 }


