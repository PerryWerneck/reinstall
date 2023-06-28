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
  * @brief Implement gtk based popup dialog.
  */

 // https://developer-old.gnome.org/gtkmm/stable/classGtk_1_1MessageDialog.html

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/ui/dialog.h>
 #include <udjat/ui/gtk/popup.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/threadpool.h>

 using namespace std;
 using namespace Udjat;

 namespace Udjat {

	Gtk::Popup::Popup(::Gtk::Window &parent, const char *message, const char *secondary) :
		::Gtk::MessageDialog{parent,String{"<b><big>",message,"</big></b>"}.c_str(),true,::Gtk::MESSAGE_INFO,::Gtk::BUTTONS_NONE,true} {

		Logger::String{"Popup:\n",message,"\n",(secondary ? secondary : "")}.trace("popup");

		set_modal(true);
		set_transient_for(parent);
		set_default_size(500,-1);

		auto message_area = get_message_area();
		message_area->set_spacing(6);

		message_area->get_style_context()->add_class("dialog-popup");
		message_area->set_border_width(12);
		message_area->set_spacing(6);

		if(secondary && *secondary) {
			set_secondary_text(secondary,true);
		}

	}

	Gtk::Popup::Popup(::Gtk::Window &parent, const Udjat::Dialog &dialog, const std::vector<Udjat::Dialog::Button> &buttons)
		:Popup(parent,dialog.message,dialog.secondary) {

		if(dialog.title && *dialog.title) {
			set_title(dialog.title);
		} else {
			set_title(parent.get_title());
		}

		for(const Udjat::Dialog::Button &button : buttons) {
			auto widget = add_button(button.label,button.id);
			switch(button.style) {
			case Udjat::Dialog::Button::Standard:
				break;

			case Udjat::Dialog::Button::Recommended:
				widget->get_style_context()->add_class("suggested-action");
				break;

			case Udjat::Dialog::Button::Destructive:
				widget->get_style_context()->add_class("destructive-action");
				break;

			}
		}

	}

	Gtk::Popup::~Popup() {
	}

	void Gtk::Popup::set(const Udjat::Dialog &dialog) {

		Glib::signal_idle().connect([this,dialog](){
			set_message(dialog.message,true);
			set_secondary_text(dialog.secondary,true);
			return 0;
		});

	}

	void Gtk::Popup::enable(int id, bool enabled) {

		Glib::signal_idle().connect([this,id,enabled](){

			set_response_sensitive(id,enabled);

			/*
			auto widget = get_widget_for_response(id);
			if(widget) {
				widget->set_sensitive(enabled);
			} else {
				Logger::String{"Can't get widget for id '",id,"'"}.error("popup");
			}
			*/

			return 0;
		});

	}

	int Gtk::Popup::run() {

		::Gtk::Dialog::show_all();
		int rc = ::Gtk::Dialog::run();
		::Gtk::Dialog::hide();

		return rc;

	}

	void Gtk::Popup::on_response(int response_id) {

		debug("Got response ",response_id);

		running = false;
		response = response_id;

		auto action_area = get_action_area();
		if(action_area) {
			action_area->set_sensitive(false);
		}

		::Gtk::MessageDialog::on_response(response_id);
	}

	int Gtk::Popup::run(const std::function<int(Udjat::Dialog::Popup &popup)> &task) {

		std::string error_message;
		auto mainloop = Glib::MainLoop::create();

		// We cant start thread before the widget initialization is complete.
		auto connection = signal_show().connect([this,task,&error_message,&mainloop]{

			// Widget is showing, start background thread.
			Udjat::ThreadPool::getInstance().push([this,task,&error_message,&mainloop](){

				usleep(100);

				try {

					int rc = task(*this);
					if(running) {
						response = rc;
					}

				} catch(const std::exception &e) {
					response = -1;
					error_message = e.what();
					Logger::String{error_message}.error("ui-task");
				} catch(...) {
					response = -1;
					error_message = _("Unexpected error on background task");
					Logger::String{error_message}.error("ui-task");
				}

				Glib::signal_idle().connect([mainloop](){
					mainloop->quit();
					return 0;
				});

			});

		});

		::Gtk::Dialog::show_all();
		mainloop->run();
		::Gtk::Dialog::hide();

		connection.disconnect();

		if(response == -1 && !error_message.empty()) {
			throw runtime_error(error_message);
		}

		return response;

	}

 }
