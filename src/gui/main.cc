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

 /*

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
 #include <unistd.h>
 #include <reinstall/action.h>
 #include <cstdlib>
 #include <locale.h>

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

 static void text_menu() {

	size_t groups = 0;
	bool console = Logger::console();

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

	Logger::console(console);
 }

 static int text_mode(bool interactive, const char *option = nullptr) {

	Logger::console(!interactive);

	Reinstall::UserInterface interface;
 	Reinstall::Controller &controller{Reinstall::Controller::getInstance()};

	controller.setup();

	try {

		if(option) {
			Reinstall::Action::set_selected(option);
		} else if(interactive) {
			text_menu();
		}

		Reinstall::Action &action = Reinstall::Action::get_selected();

		if(action.interact()) {

#ifdef _WIN32

			Reinstall::Dialog::Progress progress;

#else

			class ProgressDialog : Reinstall::Dialog::Progress {
			private:
				bool console;

				static void write(int fd, const char *str) {
					::write(fd,"\r\x1b[2K",5);
					::write(fd,str,strlen(str));
					::write(fd,"\n",1);
				}

				static void info(const char *str) {
					write(1,str);
				}

			public:
				ProgressDialog() : console{Logger::console()} {
					Logger::console(false);
				}

				~ProgressDialog() {
					Logger::console(console);
				}

				void set_title(const char *title) override {
					info(title);
				}

				void set_sub_title(const char *subtitle) override {
					info(subtitle);
				}

				void set_url(const char *url) override {
					info(url);
				}

				void set_progress(double current, double total) override {

					unsigned int cols = (unsigned int) ((current/total) * 40.0D);

					string line{"\r\x1b[7\x1b[2K"};

					line += " [";

					for(unsigned int col = 0; col < 40; col++) {
						line += (col < cols ? "#" : " ");
					}

					line += "] ";

					if(current < total) {
						line += Logger::Message{
							_("{} of {}"),
							Udjat::String{}.set_byte(current).c_str(),
							Udjat::String{}.set_byte(total).c_str()
						}.c_str();
					}

					line += "\x1b[8\r";

					::write(1,line.c_str(),line.size());
				}


			};

			ProgressDialog progress;

#endif // _WIN32

			auto builder = action.pre();
			auto writer = action.WriterFactory();

			builder->burn(writer);
			action.post(writer);

		}

	} catch(const std::exception &e) {

		write(2,"\n\n",2);
		write(2,e.what(),strlen(e.what()));
		write(2,"\n",1);

		Reinstall::Controller::getInstance().clear();
		Udjat::Application::finalize();
		return -1;

	}

	Reinstall::Controller::getInstance().clear();
	Udjat::Application::finalize();

 	return 0;
 }

 int main(int argc, char* argv[]) {

#ifdef LC_ALL
	setlocale( LC_ALL, "" );
#endif

	bind_textdomain_codeset(G_STRINGIFY(GETTEXT_PACKAGE), "UTF-8");
	textdomain(G_STRINGIFY(GETTEXT_PACKAGE));

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

		} else if(strncasecmp(ptr,"select=",7) == 0) {
			return text_mode(false,ptr+7);

		} else if(strcasecmp(ptr,"apply-default") == 0) {
			return text_mode(false);

		} else if(strncasecmp(ptr,"usb-storage-device=",19) == 0) {
			// Set usb-storage-device
			Reinstall::Writer::setUsbDeviceName(ptr+19);

		} else if(strncasecmp(ptr,"usb-storage-length=",19) == 0) {
			// Set usb-storage-device
			Reinstall::Writer::setUsbDeviceLength(Reinstall::Action::getImageSize(ptr+19));

		} else {

			const char *sep = strchr(ptr,'=');
			if(sep) {

				std::string name{ptr,(size_t) (sep-ptr)};
				std::string value{sep+1};

				Logger::String{"Setting environment ",name,"='",value,"'"}.info(PACKAGE_NAME);
				setenv(name.c_str(),value.c_str(),1);

			}

		}

	}

	return gui_mode();

 }

 */
