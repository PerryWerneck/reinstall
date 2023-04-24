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
 #include <udjat/tools/quark.h>
 #include <udjat/tools/logger.h>
 #include <private/mainwindow.h>
 #include <reinstall/controller.h>
 #include <udjat/tools/application.h>
 #include <reinstall/builder.h>
 #include <reinstall/writer.h>

 using namespace std;
 using namespace Udjat;

 static void g_syslog(const gchar *domain, GLogLevelFlags level, const gchar *message, gpointer G_GNUC_UNUSED(user_data)) {

 	static const struct Type {
 		GLogLevelFlags	level;
 		Logger::Level	lvl;
 	} types[] =
 	{
		{ G_LOG_FLAG_RECURSION,	Logger::Info		},
		{ G_LOG_FLAG_FATAL,		Logger::Error		},

		/* GLib log levels */
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
	auto app = Gtk::Application::create(PRODUCT_ID "." PACKAGE_NAME);
	MainWindow window;
	return app->run(window);
 }

 static int text_mode(bool) {

	Reinstall::UserInterface interface;
 	Reinstall::Controller &controller{Reinstall::Controller::getInstance()};

	controller.setup();

	Reinstall::Action &action = Reinstall::Action::get_selected();

	if(action.interact()) {

		Reinstall::Dialog::Progress progress;

		auto builder = action.pre();
		auto writer = action.WriterFactory();

		builder->burn(writer);
		action.post(writer);

	}

 	Reinstall::Controller::getInstance().clear();
 	Udjat::Application::finalize();

 	return 0;
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
		} else if(strcasecmp(ptr,"apply-default") == 0) {
			return text_mode(false);
		}

	}

#ifndef _WIN32
	{
		const char *ptr = strrchr(argv[0],'/');
		if(ptr) {
			ptr++;
		} else {
			ptr = argv[0];
		}

		if(!strcasecmp(ptr,"reinstall-enable")) {
			return text_mode(false);
		} else if(!strcasecmp(ptr,"reinstall-text") || !getenv("DISPLAY")) {
			return text_mode(true);
		}
	}
#endif // _WIN32

	return gui_mode();

 }
