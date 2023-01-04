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
 #include <reinstall/builder.h>
 #include <reinstall/writer.h>
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

 void MainWindow::apply() {

	Reinstall::Action &action = Reinstall::Action::get_selected();

 	g_message("Apply '%s'",std::to_string(action).c_str());

	//
	// Interact with user.
	//
	try {

		if(!action.interact()) {
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
		dialog_fail.set_title(action.get_label());
		dialog_fail.set_secondary_text(e.what());
		dialog_fail.show();
		dialog_fail.run();

		return;
	}

	//
	// Ask for confirmation.
	//
	if(action.confirmation()) {
		auto response = (Gtk::ResponseType) Dialog::Popup{
						*this,
						*action.get_button(),
						action.confirmation(),
						Gtk::MESSAGE_QUESTION,
						Gtk::BUTTONS_YES_NO
					}.run();

		if(response != Gtk::RESPONSE_YES) {
			cout << "apply\tCancelled by user" << endl;
			return;
		}

	}

	//
	// Step 1 - Prepare image get image builder.
	//
	std::shared_ptr<Reinstall::Builder> builder;
	{
		Dialog::Progress dialog;
		show(dialog);

		Udjat::ThreadPool::getInstance().push([this,&dialog,&action,&builder](){

			try {

				builder = action.pre();
				dialog.dismiss(Gtk::RESPONSE_OK);

			} catch(const std::exception &e) {

				cerr << e.what() << endl;
				failed(e.what());
				dialog.dismiss(Gtk::RESPONSE_CANCEL);

			}


		});

		if(dialog.run() != Gtk::RESPONSE_OK) {
			return;
		}

	}

	//
	// Step 2 - Another user interaction, get image writer
	//
	std::shared_ptr<Reinstall::Writer> writer;
	try {

		writer = action.WriterFactory();
		if(!writer) {
			clog << "apply\tAborting due to invalid writer" << endl;
			return;
		}

	} catch(const std::exception &e) {

		cerr << e.what() << endl;
		failed(e.what());
		return;

	}

	//
	// Step 3 - Burn image.
	//
	{
		Dialog::Progress dialog;
		show(dialog);

		Udjat::ThreadPool::getInstance().push([this,&dialog,writer,builder,&action](){

			try {

				builder->burn(writer);
				action.post(writer);
				dialog.dismiss(Gtk::RESPONSE_OK);

			} catch(const std::exception &e) {

				cerr << e.what() << endl;
				failed(e.what());
				dialog.dismiss(Gtk::RESPONSE_CANCEL);

			}

		});

		if(dialog.run() != Gtk::RESPONSE_OK) {
			return;
		}

	}

	//
	// Step 4 - Complete, notify user
	//
	{
		std::shared_ptr<Gtk::MessageDialog> popup;

		if(action.success()) {

			// Customized success dialog.

			cout << "MainWindow\tShowing action 'success' dialog" << endl;
			popup = make_shared<Dialog::Popup>(
				*this,
				*action.get_button(),
				action.success(),
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
		if(action.reboot()) {

			// Close button is the suggested action.
			auto close = popup->get_widget_for_response(Gtk::RESPONSE_CLOSE);
			close->get_style_context()->add_class("suggested-action");
			popup->set_default_response(Gtk::RESPONSE_CLOSE);

			// Reboot button is destructive.
			auto reboot = popup->add_button(_("Reboot"),Gtk::RESPONSE_APPLY);
			reboot->get_style_context()->add_class("destructive-action");

		}

		if(action.quit()) {
			auto cancel = popup->add_button(_("Quit application"),Gtk::RESPONSE_CANCEL);
			if(!action.reboot()) {
				cancel->get_style_context()->add_class("suggested-action");
				popup->set_default_response(Gtk::RESPONSE_CANCEL);
			}
		}

		popup->set_title(action.get_label());
		popup->set_default_size(500, -1);
		popup->show();
		switch(popup->run()) {
		case Gtk::RESPONSE_APPLY:
			cout << "MainWindow\tRebooting by user request" << endl;
			Gtk::Application::get_default()->quit();
			Reinstall::reboot();
			break;

		case Gtk::RESPONSE_CANCEL:
			Gtk::Application::get_default()->quit();
			break;
		}

	}
	/*
		//
		// Build and burn image.
		//

		show(dialog);


		dialog.hide(); // Just hide to wait for all enqueued state changes to run.

		//
		// Get action writer.
		//
		if(error_message.empty()) {


		}

		//
		// Burn image
		//
		if(error_message.empty()) {

			dialog.show();

			dialog.hide();

		}

		// Show last dialogs.
		{
			std::shared_ptr<Gtk::MessageDialog> popup;

			if(error_message.empty()) {


			} else if(action.failed()) {

				// Customized error dialog.

				popup = make_shared<Dialog::Popup>(
					*this,
					*action.get_button(),
					action.failed(),
					Gtk::MESSAGE_ERROR,
					Gtk::BUTTONS_OK
				);

				if(!action.failed().has_secondary()) {
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




		}
	}

	buttons.apply.set_sensitive(true);
	buttons.cancel.set_sensitive(true);
	layout.view.set_sensitive(true);

	*/
 }
