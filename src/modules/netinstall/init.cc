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
 #include <udjat/factory.h>
 #include <udjat/module.h>
 #include <udjat/tools/xml.h>

 #include <libreinstall/action.h>

 #include <libreinstall/builder.h>
 #include <libreinstall/builders/iso9660.h>

 using namespace std;
 using namespace Udjat;

 Udjat::Module * udjat_module_init() {

 	class Action : public Reinstall::Action, private iso9660::Settings {
	public:
		Action(const XML::Node &node) : Reinstall::Action{node}, iso9660::Settings{node} {

			debug("----------------------");

			/*
			// Get URL for installation kernel.
			if(!scan(node,"kernel",[this](const pugi::xml_node &node) {
				push_back(make_shared<Kernel>(node));
				return true;
			})) {
				throw runtime_error(_("Missing required entry <kernel> with the URL for installation kernel"));
			}

			// Get URL for installation init.
			if(!scan(node,"init",[this](const pugi::xml_node &node) {
				push_back(make_shared<InitRD>(node));
				return true;
			})) {
				throw runtime_error(_("Missing required entry <init> with the URL for the linuxrc program"));
			}
			*/

		}

		std::shared_ptr<Reinstall::Builder> BuilderFactory() const override {
			return iso9660::BuilderFactory(*this);
		}

 	};

 	static const Udjat::ModuleInfo moduleinfo { "Basic network install image builder" };

	class Module : public Udjat::Module, public Udjat::Factory {
	public:
		Module() : Udjat::Module("network-installer", moduleinfo), Udjat::Factory("network-installer",moduleinfo) {
		}

		bool generic(const XML::Node &node) override {
			Reinstall::push_back(make_shared<Action>(node));
			return true;
		}

	};

	return new Module();

 }


 /*
 #include <udjat/module.h>
 #include <udjat/factory.h>
 #include <udjat/tools/object.h>
 #include <stdexcept>
 #include <reinstall/actions/isobuilder.h>
 #include <reinstall/writer.h>
 #include <reinstall/group.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <reinstall/sources/kernel.h>
 #include <reinstall/sources/initrd.h>

 using namespace std;
 using namespace Udjat;

 Udjat::Module * udjat_module_init() {

 	static const Udjat::ModuleInfo moduleinfo { "Basic network install image builder" };

	class Module : public Udjat::Module, public Udjat::Factory {
	public:
		Module() : Udjat::Module("network-installer", moduleinfo), Udjat::Factory("network-installer",moduleinfo) {
		}

		bool generic(const pugi::xml_node &node) override {

			class Action : public Reinstall::IsoBuilder {
			public:
				Action(const pugi::xml_node &node) : Reinstall::IsoBuilder(node,"drive-removable-media") {

					// Get URL for installation kernel.
					if(!scan(node,"kernel",[this](const pugi::xml_node &node) {
						push_back(make_shared<Reinstall::Kernel>(node));
						return true;
					})) {
						throw runtime_error(_("Missing required entry <kernel> with the URL for installation kernel"));
					}

					// Get URL for installation init.
					if(!scan(node,"init",[this](const pugi::xml_node &node) {
						push_back(make_shared<Reinstall::InitRD>(node));
						return true;
					})) {
						throw runtime_error(_("Missing required entry <init> with the URL for the linuxrc program"));
					}

				}

				virtual ~Action() {
				}

			};

			Reinstall::push_back(node,make_shared<Action>(node));

			return true;
		}

	};

	return new Module();

 }
 */

