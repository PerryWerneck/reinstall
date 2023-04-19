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
 #include <reinstall/script.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>

 #include <unistd.h>
 #include <pwd.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

 	static int getuid(const pugi::xml_node &node) {

 		const char *user = node.attribute("user").as_string("");

 		if(!(user && *user)) {
			return -1;
 		}

		size_t szBuffer = sysconf(_SC_GETPW_R_SIZE_MAX);
		if(szBuffer == (size_t) -1) {
			szBuffer = 16384;
		}

		char buffer[szBuffer+1];
		memset(buffer,0,szBuffer+1);

		struct passwd pwd;
		struct passwd *result;

		if(getpwnam_r(user, &pwd, buffer, szBuffer, &result) != 0) {
			throw system_error(errno,system_category(),user);
		};

		if(!result) {
			throw system_error(ENOENT,system_category(),user);
		}

		return (int) result->pw_uid;

 	}

	Script::Script(const pugi::xml_node &node) : cmdline{Quark(node,"cmdline","").c_str()},uid{getuid(node)} {
		if(!(cmdline && *cmdline)) {
			throw runtime_error(_("The required attribute 'cmdline' is missing"));
		}
	}

	Script::~Script() {
	}

	int Script::run(const Udjat::NamedObject &object) {

		class SubProcess : public Udjat::SubProcess {
		protected:

			void onStdOut(const char *line) override {
				Logger::String{line}.write(Logger::Trace,name());
			}

			void onStdErr(const char *line) override {
				error() << line << endl;
			}

		public:
			SubProcess(const NamedObject &obj, const Udjat::String &command) : Udjat::SubProcess(obj.name(),command.c_str()) {
			}

		};

		return SubProcess{object,String{cmdline}.expand(object)}.run();

	}

 }

