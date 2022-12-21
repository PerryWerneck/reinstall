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
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/application.h>
 #include <reinstall/controller.h>
 #include <udjat/tools/logger.h>

 using namespace Udjat;

 MainWindow::MainWindow() {

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
	layout.title.get_style_context()->add_class("maintitle");
	layout.vbox.pack_start(layout.title,false,false,3);

	layout.swindow.set_hexpand(true);
	layout.swindow.set_vexpand(true);
	layout.swindow.add(layout.view);
	layout.vbox.add(layout.swindow);

	layout.bbox.set_layout(Gtk::BUTTONBOX_END);
	layout.bbox.add(layout.cancel);
	layout.bbox.add(layout.apply);
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
	Udjat::Application::finalize();
 }

 void MainWindow::on_show() {

	Gtk::Window::on_show();

	// Load options
	{
		Dialog::Progress dialog;
		dialog.set_title(get_title());
		dialog.set_parent(*this);
		dialog.sub_title() = _("Getting configuration");
		dialog.icon().hide();
		dialog.footer(false);
		dialog.show();

		Udjat::ThreadPool::getInstance().push([&dialog](){

			// First get controller to construct the factories.
			Reinstall::Controller::getInstance();

			// Load image definitions.
			Udjat::Application::setup("./xml.d",true);

			// And dismiss dialog.
			dialog.dismiss();

		});

		dialog.run();
	}

	// Create groups.
	Reinstall::Controller::getInstance().for_each([this](std::shared_ptr<Reinstall::Group> group){

		layout.view.pack_start(group->title,false,false,0);
		if(group->subtitle) {
			layout.view.pack_start(group->subtitle,false,false,0);
		}


		return false;
	});

	layout.view.show_all();

 }
