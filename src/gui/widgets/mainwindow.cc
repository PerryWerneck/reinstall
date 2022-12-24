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

 using namespace Udjat;

 MainWindow::MainWindow() {

 	{
		auto css = Gtk::CssProvider::create();
		css->load_from_path("./stylesheet.css");
		get_style_context()->add_provider_for_screen(Gdk::Screen::get_default(), css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
 	}

	set_title(_("System reinstallattion"));
	set_default_size(600, 400);

	// Add separator
	gtk_box_pack_start(
			GTK_BOX(layout.hbox.gobj()),
			gtk_separator_new(GTK_ORIENTATION_VERTICAL),
			FALSE,
			FALSE,
			0);

	// https://developer-old.gnome.org/gtkmm-tutorial/stable/sec-clipboard-examples.html.en
	// http://transit.iut2.upmf-grenoble.fr/doc/gtkmm-3.0/tutorial/html/sec-helloworld.html

	layout.vbox.set_hexpand(true);
	layout.vbox.set_vexpand(true);
	layout.vbox.set_border_width(6);

	// A wide variety of style classes may be applied to labels, such as .title, .subtitle, .dim-label, etc
	layout.title.get_style_context()->add_class("main-title");
	layout.vbox.pack_start(layout.title,false,false,3);

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

	add(layout.hbox);
	layout.hbox.show_all();

 }

 MainWindow::~MainWindow() {
 	Reinstall::Controller::getInstance().clear();
	Udjat::Application::finalize();
 }

 void MainWindow::on_show() {

	// Load options
	{
		Dialog::Progress dialog;
		dialog.set_title(get_title());
		dialog.set_parent(*this);
		dialog.sub_title() = _("Getting configuration");
		dialog.icon().hide();
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

	// Create groups.
	Reinstall::Controller::getInstance().for_each([this](std::shared_ptr<Reinstall::Group> group){

		auto box = new Gtk::Box(Gtk::ORIENTATION_VERTICAL);
		box->get_style_context()->add_class("group-box");

		debug("Adding option ",std::to_string(group->title));

		auto grid = new Gtk::Grid();
		grid->get_style_context()->add_class("group-title-box");

		group->title.get_style_context()->add_class("group-title");
		grid->attach(group->title,0,0,1,1);
		if(group->subtitle) {
			group->subtitle.get_style_context()->add_class("group-subtitle");
			grid->attach(group->subtitle,0,1,1,1);
		}

		box->add(*grid);

		group->for_each([this,group,box](std::shared_ptr<Reinstall::Action> action) {

			debug("Adding option ",std::to_string(group->title),"/",std::to_string(action->title));
			::Widget::Action *button = new ::Widget::Action(action);

			button->set_halign(Gtk::ALIGN_FILL);
			button->get_style_context()->add_class("action-inactive");
			box->pack_start(*button,true,true,0);

			button->get_style_context()->add_class(button->get_active() ? "action-active" : "action-inactive");

			button->signal_toggled().connect([&,button,action]() {

				if(button->get_active()) {
					selected = action;
					debug("Action '",std::to_string(selected->title),"' is now enabled");
					buttons.apply.set_sensitive(true);
					button->get_style_context()->remove_class("action-inactive");
					button->get_style_context()->add_class("action-active");
				} else {
					button->get_style_context()->remove_class("action-active");
					button->get_style_context()->add_class("action-inactive");
				}

			});

			return false;

		});

		layout.view.pack_start(*box,false,false,0);
		return false;
	});

	layout.view.show_all();

	Gtk::Window::on_show();

 }

 void MainWindow::apply() {

 	g_message("Apply '%s' action",std::to_string(selected->title).c_str());
	buttons.apply.set_sensitive(false);
	buttons.cancel.set_sensitive(false);
	layout.view.set_sensitive(false);

	// Execute action
	{
		Dialog::Progress dialog;
		dialog.set_parent(*this);
		dialog.set_decorated(false);
		dialog.set_deletable(false);
		dialog.show();

		Udjat::ThreadPool::getInstance().push([&dialog,this](){


			sleep(5);
			dialog.dismiss();

		});

		dialog.run();
	}

	buttons.apply.set_sensitive(true);
	buttons.cancel.set_sensitive(true);
	layout.view.set_sensitive(true);
 }
