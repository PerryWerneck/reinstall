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
 #include <reinstall/action.h>
 #include <udjat/tools/application.h>
 #include <reinstall/builder.h>
 #include <reinstall/writer.h>
 #include <cstdio>
 #include <readline/readline.h>

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

 static unsigned short get_option() {

	static const char *prompt = _("\nSelect option: ");
	std::string str;

	write(1,prompt,strlen(prompt));
	std::getline(cin,str);

	unsigned short rc = atoi(str.c_str());

	if(!rc) {
		throw runtime_error(_("Cancelled by user"));
	}

	return rc;
 }

 static bool text_menu() {

	try {

		size_t groups = 0;

		write(1,"\n",1);
		Reinstall::Controller::getInstance().for_each([&groups](std::shared_ptr<Reinstall::Abstract::Group> group) {

			String option{group->id," - ",group->get_label(),"\n"};
			write(1,option.c_str(),option.size());
			groups++;

			return false;
		});

		static const char * clear_and_up{"\r\33[1A\r\33[2K"};
		short option = get_option();

		for(size_t ln = 0; ln < (groups+2); ln++) {
			write(1,clear_and_up,strlen(clear_and_up));
		}

		auto group = Reinstall::Abstract::Group::find(option);

		{
			String option{group->get_label(),":\n\n"};
			write(1,option.c_str(),option.size());
		}

		// Select action.
		size_t actions = 0;
		group->for_each([&actions](std::shared_ptr<Reinstall::Action> action){

			String option{"  ",action->id," - ",action->get_label(),"\n"};
			write(1,option.c_str(),option.size());
			actions++;

			return false;
		});

		option = get_option();

		for(size_t ln = 0; ln < (actions+4); ln++) {
			write(1,clear_and_up,strlen(clear_and_up));
		}

		if(!group->for_each([option](std::shared_ptr<Reinstall::Action> action){

			if(action->id == option) {
				action->set_selected();
				return true;
			}

			return false;
		})) {
			throw runtime_error(_("Invalid action id"));
		};

		{
			String option{"\33[1m",group->get_label(), " - ", Reinstall::Action::get_selected().get_label(), "\33[0m\n\n"};
			write(1,option.c_str(),option.size());
		}

	} catch(const std::exception &e) {

		write(2,"\n\n",2);
		write(2,e.what(),strlen(e.what()));
		write(2,"\n",2);
		return false;

	}

	Logger::console(true);
	return true;
 }

 static int text_mode(bool interactive) {

	Logger::console(!interactive);

	Reinstall::UserInterface interface;
 	Reinstall::Controller &controller{Reinstall::Controller::getInstance()};

	controller.setup();

	if(interactive) {

		if(!text_menu()) {
			Reinstall::Controller::getInstance().clear();
			Udjat::Application::finalize();
			return -1;
		}

	}

	Reinstall::Action &action = Reinstall::Action::get_selected();

	if(action.interact()) {

		Reinstall::Dialog::Progress progress;

		auto builder = action.pre();
		auto writer = action.WriterFactory();

		builder->burn(writer);
		action.post(writer);

	}


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
		} else if(strcasecmp(ptr,"text") == 0 || strcasecmp(ptr,"t") == 0) {
			return text_mode(true);
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
