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
 #include <stdexcept>
 #include <reinstall/actions/isobuilder.h>
 #include <reinstall/group.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/logger.h>

 using namespace std;
 using namespace Udjat;

 Udjat::Module * udjat_module_init() {

 	static const Udjat::ModuleInfo moduleinfo { "IsoWriter module" };

	class Module : public Udjat::Module, public Udjat::Factory {
	public:
		Module() : Udjat::Module("IsoWriter", moduleinfo), Udjat::Factory("iso-writer",moduleinfo) {
		}

		bool push_back(const pugi::xml_node &node) override {

			class Action : public Reinstall::IsoBuilder {
			private:
				const char *path = nullptr;

			public:
				Action(const pugi::xml_node &node) : Reinstall::IsoBuilder(node), path{Quark{node,"iso-filename"}.c_str()} {

					debug("Creating iso-writer action '",name(),"'");

					if(!(path && *path)) {
						throw runtime_error("Required attribute 'iso-filename' is missing");
					}

					if(!(icon_name && *icon_name)) {
						// https://specifications.freedesktop.org/icon-naming-spec/latest/
						// drive-removable-media
						icon_name = "document-save-as";
					}

				}

				virtual ~Action() {
				}

				void write(Reinstall::iso9660::Worker &worker) override {
					worker.save(path);
					post(path);
				}

			};

			Reinstall::push_back(node,make_shared<Action>(node));

			//Reinstall::Abstract::Group::find(node)->push_back(make_shared<Action>(node));

			return true;
		}

	};

	return new Module();

 }

