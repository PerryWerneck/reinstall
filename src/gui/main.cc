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
 #include <gtkmm.h>
 #include <glibmm/i18n.h>
 #include <iostream>
 #include <private/dialogs.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/logger.h>
 #include <memory>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/application.h>
 #include <reinstall/controller.h>

 using namespace Gtk;
 using namespace std;
 using namespace Udjat;

 int main(int argc, char* argv[]) {

	class MainWindow : public Window {
	private:

		Label title{ _( "Select option" ), ALIGN_START };
		Box hbox, vbox{ORIENTATION_VERTICAL};
		ScrolledWindow view;
		ButtonBox bbox;
		Button apply{_("_Apply"), true}, cancel{_("_Cancel"), true};

	public:
		MainWindow() {

			set_title(PACKAGE_STRING);
			set_default_size(600, 400);

			// https://developer-old.gnome.org/gtkmm-tutorial/stable/sec-clipboard-examples.html.en
			// http://transit.iut2.upmf-grenoble.fr/doc/gtkmm-3.0/tutorial/html/sec-helloworld.html

			vbox.set_hexpand(true);
			vbox.set_vexpand(true);
			vbox.set_border_width(6);

			// A wide variety of style classes may be applied to labels, such as .title, .subtitle, .dim-label, etc
			title.get_style_context()->add_class("maintitle");
			vbox.pack_start(title,false,false,3);

			view.set_hexpand(true);
			view.set_vexpand(true);
			vbox.add(view);

			bbox.set_layout(BUTTONBOX_END);
			bbox.add(cancel);
			bbox.add(apply);
			bbox.set_hexpand(true);
			vbox.add(bbox);

			bbox.set_spacing(6);

			hbox.set_hexpand(true);
			hbox.set_vexpand(true);
			hbox.add(vbox);
			add(hbox);
			show_all();

			// Initialize
			{
				Dialog::Progress dialog;

				dialog.set_title(get_title());
				dialog.set_parent(*this);
				dialog.sub_title() = _("Wait, initializing application...");
				dialog.footer(false);
				dialog.show();

				ThreadPool::getInstance().push([&dialog](){

					// First get controller to construct the factories.
					Reinstall::Controller::getInstance();

#ifdef DEBUG
					Udjat::Application::setup("./xml.d",true);
#else
					#error TODO!
#endif // DEBUG

					dialog.dismiss();
				});

				dialog.run();
			}

		}
	};

	Udjat::Quark::init(argc,argv);

	Udjat::Logger::redirect();

	auto app = Gtk::Application::create("br.com.bb.reinstall");

	MainWindow window;
	window.set_default_size(200, 200);

	// Dialog::Progress progress;
	// progress.show();

	return app->run(window);
 }
