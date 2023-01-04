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
 #include <udjat/tools/object.h>
 #include <stdexcept>
 #include <reinstall/userinterface.h>
 #include <reinstall/group.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <reinstall/actions/kernel.h>
 #include <reinstall/actions/initrd.h>

 /*

 */

 using namespace std;
 using namespace Udjat;

 Udjat::Module * udjat_module_init() {

 	static const Udjat::ModuleInfo moduleinfo { "Grub based system installation" };

	class Module : public Udjat::Module, public Udjat::Factory {
	public:
		Module() : Udjat::Module("local-installer", moduleinfo), Udjat::Factory("local-installer",moduleinfo) {
		}

		bool push_back(const pugi::xml_node &node) override {

			class Action : public Reinstall::Action {
			private:
				std::shared_ptr<Reinstall::Kernel> kernel;
				std::shared_ptr<Reinstall::InitRD> initrd;

			public:
				Action(const pugi::xml_node &node) : Reinstall::Action(node,"computer") {

					// Get URL for installation kernel.
					if(!scan(node,"kernel",[this](const pugi::xml_node &node) {
						auto source = make_shared<Reinstall::Kernel>(node);
						if(!(source->filename && source->filename[0])) {
							source->filename = "${boot}/kernel.install";
						}
						push_back(source);
						return true;
					})) {
						throw runtime_error(_("Missing required entry <kernel> with the URL for installation kernel"));
					}

					// Get URL for installation init.
					if(!scan(node,"init",[this](const pugi::xml_node &node) {
						auto source = make_shared<Reinstall::InitRD>(node);
						if(!(source->filename && source->filename[0])) {
							source->filename = "${boot}/initrd.install";
						}
						push_back(source);
						return true;
					})) {
						throw runtime_error(_("Missing required entry <init> with the URL for the linuxrc program"));
					}

				}

				bool getProperty(const char *key, std::string &value) const noexcept override {

					if(strcasecmp(key,"install-enabled") == 0) {
						value = "1";
						return true;
					}

					if(strcasecmp(key,"kernel-path") == 0) {
						return true;
					}

					if(strcasecmp(key,"initrd-path") == 0) {
						return true;
					}

					return Reinstall::Action::getProperty(key,value);
				}

			};

			Reinstall::push_back(node,make_shared<Action>(node));

			return true;
		}

	};

	return new Module();

 }

