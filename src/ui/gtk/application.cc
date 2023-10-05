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
  * @brief Brief description of this source.
  */

 #include <config.h>

 #include <gtkmm.h>
 #include <glibmm/i18n.h>

 #include <stdexcept>

 #include <udjat/defs.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/threadpool.h>

 #include <udjat/ui/gtk/application.h>
 #include <udjat/ui/gtk/progress.h>

 using namespace std;

 static void g_syslog(const gchar *domain, GLogLevelFlags level, const gchar *message, gpointer G_GNUC_UNUSED(user_data)) {

 	static const struct Type {
 		GLogLevelFlags			level;
 		Udjat::Logger::Level	lvl;
 	} types[] =
 	{
		{ G_LOG_FLAG_RECURSION,	Udjat::Logger::Info			},
		{ G_LOG_FLAG_FATAL,		Udjat::Logger::Error		},

		// GLib log levels
		{ G_LOG_LEVEL_ERROR,	Udjat::Logger::Error		},
		{ G_LOG_LEVEL_CRITICAL,	Udjat::Logger::Error		},
		{ G_LOG_LEVEL_WARNING,	Udjat::Logger::Warning		},
		{ G_LOG_LEVEL_MESSAGE,	Udjat::Logger::Info			},
		{ G_LOG_LEVEL_INFO,		Udjat::Logger::Info			},
		{ G_LOG_LEVEL_DEBUG,	Udjat::Logger::Debug		},
 	};

	for(size_t ix=0; ix < G_N_ELEMENTS(types);ix++) {
		if(types[ix].level == level) {
			Udjat::Logger::String{message}.write(types[ix].lvl,domain ? domain : "gtk");
			return;
		}
	}

	Udjat::Logger::String{message}.error(domain ? domain : "gtk");

 }

 namespace Udjat {

	Glib::RefPtr<Udjat::Gtk::Application> Udjat::Gtk::Application::create(const char *application_id) {

		auto application = Glib::RefPtr<Udjat::Gtk::Application>{new Udjat::Gtk::Application(Glib::ustring{application_id})};
		set_default(application);
		return application;

	}

	Gtk::Application::Application(const Glib::ustring& application_id) : ::Gtk::Application{application_id} {
		g_log_set_default_handler(g_syslog,NULL);
	}

	Gtk::Application::~Application() {
		unset_default();
	}

	int Gtk::Application::run(const char *definitions) {

		::Gtk::MessageDialog dialog{
			_("Initializing application"),
			true,
			::Gtk::MESSAGE_INFO,
			::Gtk::BUTTONS_NONE,
			true
		};

		dialog.set_default_size(500,-1);

		std::string error_message;
		int rc = -1;
		auto mainloop = Glib::MainLoop::create();

		auto connection = dialog.signal_show().connect([this,&rc,definitions,&error_message,&mainloop]{

			// Widget is showing, start background thread.
			Udjat::ThreadPool::getInstance().push([this,&rc,definitions,&error_message,&mainloop](){

				usleep(100);

				try {

					rc = init(definitions);

				} catch(const std::exception &e) {
					rc = -1;
					error_message = e.what();
					Logger::String{error_message}.error("ui");
				} catch(...) {
					rc = -1;
					error_message = _("Unexpected error on background task");
					Logger::String{error_message}.error("ui");
				}

				Glib::signal_idle().connect([mainloop](){
					mainloop->quit();
					return 0;
				});

			});

		});

		dialog.show_all();
		mainloop->run();
		dialog.hide();

		if(rc) {

			Logger::String{"Initialization procedure has finished with rc=",rc}.error(name());

			::Gtk::MessageDialog dialog_fail{
				_("The initialization procedure has failed, the application cant continue"),
				false,
				::Gtk::MESSAGE_ERROR,
				::Gtk::BUTTONS_CLOSE,
				true
			};

			dialog_fail.set_default_size(500, -1);
			if(!error_message.empty()) {
 				dialog_fail.set_secondary_text(error_message);
			}
			dialog_fail.show();
			dialog_fail.run();

			return rc;

		}

		return ::Gtk::Application::run(0,NULL);

	}


 }
