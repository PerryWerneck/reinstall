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
 #include <reinstall/source.h>

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

 int main(int argc, char* argv[]) {

	Udjat::Quark::init();
	Udjat::Logger::redirect();

#ifdef DEBUG
	Udjat::Logger::enable(Udjat::Logger::Trace);
	Udjat::Logger::enable(Udjat::Logger::Debug);
#endif // DEBUG

	g_log_set_default_handler(g_syslog,NULL);

	auto app = Gtk::Application::create(PRODUCT_ID "." PACKAGE_NAME);
	MainWindow window;

	return app->run(window);

 }
