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
 #include <udjat/module.h>
 #include <udjat/factory.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <reinstall/action.h>
 #include <reinstall/source.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/protocol.h>
 #include <iostream>
 #include <reinstall/dialogs/progress.h>
 #include <reinstall/builder.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <reinstall/sources/cached.h>
 #include <reinstall/actions/isobuilder.h>

 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 using namespace std;
 using namespace Udjat;
 using namespace Reinstall;

 Udjat::Module * udjat_module_init() {

 	static const Udjat::ModuleInfo moduleinfo { "IsoBuilder module" };

	class Module : public Udjat::Module, public Udjat::Factory {
	public:
		Module() : Udjat::Module("IsoBuilder", moduleinfo), Udjat::Factory("iso-builder",moduleinfo) {
		}

		bool generic(const pugi::xml_node &node) override {

			/// @brief ISO Writer Action
			class Action : public Reinstall::Action {
			public:
				Action(const pugi::xml_node &node) : Reinstall::Action(node,"drive-removable-media") {


				}

				std::shared_ptr<Reinstall::Builder> BuilderFactory() override {

					class Builder : public Reinstall::Builder {
					private:
						int fd = -1;

					public:
						Builder() = default;

						~Builder() {
							if(fd > 0) {
								debug("Closing device");
								::close(fd);
								debug("Device closed");
								fd = -1;
							}
						}

						void pre(const Reinstall::Action &action) override {
							debug(__FUNCTION__);
						}

						void build(const Reinstall::Action &action) override {
							debug(__FUNCTION__);
						}

						void post(const Reinstall::Action &action) override {
							debug(__FUNCTION__);
						}

					};

					return(make_shared<Builder>());

				}

			};

			Reinstall::push_back(node,make_shared<Action>(node));
			return true;

		}

	};

	return new Module();

 }

