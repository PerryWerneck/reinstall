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
 #include <udjat/tools/application.h>
 #include <udjat/tools/protocol.h>
 #include <iostream>
 #include <reinstall/dialogs/progress.h>
 #include <reinstall/builder.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>

 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 using namespace std;
 using namespace Udjat;

 Udjat::Module * udjat_module_init() {

 	static const Udjat::ModuleInfo moduleinfo { "IsoWriter module" };

	class Module : public Udjat::Module, public Udjat::Factory {
	public:
		Module() : Udjat::Module("IsoWriter", moduleinfo), Udjat::Factory("iso-writer",moduleinfo) {
		}

		bool push_back(const pugi::xml_node &node) override {

			/// @brief ISO Writer Action
			class Action : public Reinstall::Action {
			public:
				Action(const pugi::xml_node &node) : Reinstall::Action(node,"drive-removable-media") {

					/// @brief ISO Image source
					class Source : public Reinstall::Source {
					private:
						bool cache;

					public:
						Source(const pugi::xml_node &node) : Reinstall::Source(node), cache{getAttribute(node,"cache",true)} {
							if(!(url && *url)) {
								throw runtime_error(_("Required attribute 'URL' is missing"));
							}
							if(!(message && *message)) {
								message = _("Downloading ISO image");
							}
						}

						std::string save() override {

							if(cache) {
								// Build cache filename.
								this->filename = Quark{Udjat::Application::CacheDir("iso").build_filename(url)}.c_str();
							}

							return Reinstall::Source::save();
						}

					};

					sources.clear(); // Remove other sources.
					sources.insert(make_shared<Source>(node));

				}

			};

			Reinstall::push_back(node,make_shared<Action>(node));
			return true;

		}

	};

	return new Module();

 }

