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

 #include <udjat/defs.h>
 #include <udjat/ui/gtk/application.h>

 #include <private/mainwindow.h>

 int main(int argc, char* argv[]) {

	/// @brief The application object.
 	class Application : public Udjat::Gtk::Application {
	private:
		Application(const char *application_id) : Udjat::Gtk::Application{application_id} {
		}

	public:
		static Glib::RefPtr<Application> create() {

#ifdef DEBUG
			Udjat::Logger::enable(Udjat::Logger::Trace);
			Udjat::Logger::enable(Udjat::Logger::Debug);
			Udjat::Logger::console(true);
#endif // DEBUG

			auto application = Glib::RefPtr<Application>{new Application(PRODUCT_ID "." PACKAGE_NAME)};
			set_default(application);
			return application;

		}

 	};

 	return Application::create()->run(argc,argv);

 }
 /*
 using namespace std;
 using namespace Udjat;
 using namespace ::Gtk;

 static void g_syslog(const gchar *domain, GLogLevelFlags level, const gchar *message, gpointer G_GNUC_UNUSED(user_data)) {

 	static const struct Type {
 		GLogLevelFlags	level;
 		Logger::Level	lvl;
 	} types[] =
 	{
		{ G_LOG_FLAG_RECURSION,	Logger::Info		},
		{ G_LOG_FLAG_FATAL,		Logger::Error		},

		// GLib log levels
		{ G_LOG_LEVEL_ERROR,	Logger::Error		},
		{ G_LOG_LEVEL_CRITICAL,	Logger::Error		},
		{ G_LOG_LEVEL_WARNING,	Logger::Warning		},
		{ G_LOG_LEVEL_MESSAGE,	Logger::Info		},
		{ G_LOG_LEVEL_INFO,		Logger::Info		},
		{ G_LOG_LEVEL_DEBUG,	Logger::Debug		},
 	};

	for(size_t ix=0; ix < G_N_ELEMENTS(types);ix++) {
		if(types[ix].level == level) {
			Logger::String{message}.write(types[ix].lvl,domain ? domain : "gtk");
			return;
		}
	}

	cerr << message << endl;
 }

 static int gui_mode() {
	auto app = ::Gtk::Application::create(PRODUCT_ID "." PACKAGE_NAME);
	MainWindow window;
	return app->run(window);
 }

 int main(int argc, char* argv[]) {

	Udjat::Quark::init();
	Udjat::Logger::redirect();

#ifdef DEBUG
	Udjat::Logger::enable(Udjat::Logger::Trace);
	Udjat::Logger::enable(Udjat::Logger::Debug);
	Udjat::Logger::console(true);
#else
	Udjat::Logger::console(false);
#endif // DEBUG

	g_log_set_default_handler(g_syslog,NULL);

	for(int ix = 1; ix < argc; ix++) {

		const char *ptr = argv[ix];
		while(*ptr && *ptr == '-') {
			ptr++;
		}

		if(strcasecmp(ptr,"foreground") == 0 || strcasecmp(ptr,"f") == 0 || strcasecmp(ptr,"verbose") == 0 || strcasecmp(ptr,"v") == 0) {
			Udjat::Logger::console(true);

		} else if(strcasecmp(ptr,"quiet") == 0 || strcasecmp(ptr,"q") == 0) {
			Udjat::Logger::console(false);

		} else if(strcasecmp(ptr,"text") == 0 || strcasecmp(ptr,"t") == 0) {
			return text_mode(true);

		} else if(strcasecmp(ptr,"apply-default") == 0) {
			return text_mode(false);

		} else if(strncasecmp(ptr,"usb-storage-device=",19) == 0) {
			// Set usb-storage-device
			Reinstall::Writer::setUsbDeviceName(ptr+19);

		} else if(strncasecmp(ptr,"usb-storage-length=",19) == 0) {
			// Set usb-storage-device
			Reinstall::Writer::setUsbDeviceLength(Reinstall::Action::getImageSize(ptr+19));

		}

	}

	return gui_mode();

 }
 */
