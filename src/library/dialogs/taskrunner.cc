/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2022 Perry Werneck <perry.werneck@gmail.com>
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
 #include <reinstall/dialogs/taskrunner.h>
 #include <reinstall/userinterface.h>
 #include <memory>
 #include <iostream>
 #include <udjat/tools/logger.h>
 #include <unistd.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	static void write(int fd, const char *str) {
		::write(fd,str,strlen(str));
		::write(fd,"\n",1);
	}

	static void info(const char *str) {
		write(1,str);
	}

	static void error(const char *str) {
		write(2,str);
	}

	std::shared_ptr<Dialog::TaskRunner> UserInterface::TaskRunnerFactory(const char *message, bool markup) {

		class TaskRunner : public Dialog::TaskRunner {
		private:
			bool console;

		public:
			TaskRunner(const char *message, bool markup) : console{Logger::console()} {
				Logger::console(false);
				info("\n");
				set_title(message,markup);
			}

			virtual ~TaskRunner() {
				Logger::console(console);
			}

		};

		return make_shared<TaskRunner>(message,markup);
	}

	Dialog::TaskRunner::TaskRunner() {
	}

	Dialog::TaskRunner::~TaskRunner() {
	}

	void Dialog::TaskRunner::show() {
	}

	int Dialog::TaskRunner::push(const std::function<int()> &callback, bool s) {

		debug("Background task begin");
		int response = -1;

		if(s) {
			show();
		}

		try {

			response = callback();

		} catch(std::system_error &e) {

			response = - e.code().value();
			Logger::String{e.what()," (rc=",response,")"}.error();
			error(e.what());

		} catch(std::exception &e) {

			response = -1;
			Logger::String{e.what()}.error();
			error(e.what());

		} catch(...) {

			response = -1;

			Logger::String message{"Unexpected error running background task"};

			message.error();
			error(message.c_str());

		}

		debug("Background task end");

		return response;
	}

	void Dialog::TaskRunner::set(const Action UDJAT_UNUSED(&action)) {
	}

	static void display(const char *str, bool markup) {

		if(markup) {

			static const struct {
				const char *from;
				const char *to;
			} xlat[] = {
				{ "<b>", 	"\x1b[1m"	},
				{ "</b>",	"\x1b[0m"	}
			};

			string text{str};

			for(size_t ix=0; ix < N_ELEMENTS(xlat); ix++) {
				const char *ptr = strcasestr(text.c_str(),xlat[ix].from);
				if(ptr) {
					text.replace((ptr - text.c_str()),strlen(xlat[ix].from),xlat[ix].to);
				}
			}

			info(text.c_str());
		} else {
			info(str);
		}

	}

	void Dialog::TaskRunner::set_title(const char *text, bool markup) {
		display(text,markup);
	}

	void Dialog::TaskRunner::set_sub_title(const char *text, bool markup) {
		display(text,markup);
	}

	void Dialog::TaskRunner::set(const Dialog::Popup &popup) {
		if(popup.message && *popup.message) {
			set_title(popup.message);
		}
		if(popup.secondary && *popup.secondary) {
			set_sub_title(popup.secondary);
		}
	}

	void Dialog::Button::enable(bool enabled) {
		if(enabled) {
			activate();
		}
	}

	std::shared_ptr<Dialog::Button> Dialog::TaskRunner::ButtonFactory(const char *label, const std::function<void()> &callback) {

		class Button : public Dialog::Button {
		private:
			const std::function<void()> callback;

		public:
			Button(const std::function<void()> &c) : callback{c} {
			}

			void activate() override {
				callback();
			}

		};

		return make_shared<Button>(callback);

	}


 }
