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
  * @brief Implement the application main window.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <gtkmm.h>
 #include <glibmm/i18n.h>
 #include <private/mainwindow.h>
 #include <udjat/tools/xml.h>
 #include <udjat/ui/menu.h>
 #include <udjat/moduleinfo.h>
 #include <udjat/ui/dialogs/progress.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/factory.h>
 #include <string.h>

 using namespace Udjat;
 using namespace Gtk;
 using namespace std;

 static const ModuleInfo moduleinfo{PACKAGE_NAME " Main window"};

 MainWindow::MainWindow() : Factory{"MainWindow",moduleinfo} {

 	{
		auto css = Gtk::CssProvider::create();
#ifdef DEBUG
		css->load_from_path("./stylesheet.css");
#else
		css->load_from_path(Application::DataFile("stylesheet.css").c_str());
#endif // DEBUG
		get_style_context()->add_provider_for_screen(Gdk::Screen::get_default(), css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
 	}

 	set_title(Config::Value<string>("MainWindow","title",_("System Installer")));
	set_default_size(800, 600);

	/*
	// MainWindow logo
#ifdef DEBUG
	set_icon(Config::Value<string>{"MainWindow","icon","./icon.svg"}.c_str());
#else
	set_icon(Config::Value<string>{"MainWindow","icon",PRODUCT_ID "." PACKAGE_NAME}.c_str());
#endif // DEBUG
	*/


	// Left box
	{
		Gtk::Box * box = new Gtk::Box(Gtk::ORIENTATION_VERTICAL);
		box->get_style_context()->add_class("sidebar");

		box->set_hexpand(false);
		box->set_vexpand(true);
		// box->add(logo);

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

		// TODO: Activate selected option

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
 	Udjat::Application::finalize();
 }

 void MainWindow::set_icon_name(const char *icon_name) {

 }

 void MainWindow::on_show() {

	Gtk::Window::on_show();

	auto rc = Udjat::Dialog::Controller::run([](Udjat::Dialog::Progress &progress){

		progress.title(_("Initializing application"));
		progress.message(_("Loading configuration"));
		progress.pulse();

		sleep(5);

		return 0;
	});

	if(rc) {

		// The initialization has failed.
		Logger::String{"Initialization procedure has finished with rc=",rc}.error("MainWindow");


	}

 }

 bool MainWindow::generic(const XML::Node &node) {
 }

 void MainWindow::push_back(const Menu::Item *menu, const XML::Node &node) {
 }

 void MainWindow::remove(const Udjat::Menu::Item *menu) {
 }
