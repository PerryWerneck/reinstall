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
 #include <reinstall/actions/isobuilder.h>
 #include <reinstall/userinterface.h>
 #include <reinstall/writer.h>
 #include <reinstall/group.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>

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
				const char *filename;
				std::string isoname;

			public:
				Action(const pugi::xml_node &node) : Reinstall::IsoBuilder(node), filename{getAttribute(node,"filename","")} {

					debug("Creating iso-writer action '",name(),"'");

					if(!(icon_name && *icon_name)) {
						// https://specifications.freedesktop.org/icon-naming-spec/latest/
						// drive-removable-media
						icon_name = "document-save-as";
					}

				}

				virtual ~Action() {
				}

				bool interact() override {

					isoname = Reinstall::UserInterface::getInstance().FilenameFactory(
						_("Select target image file"),
						_("Image file name"),
						_("_Save"),
						filename,
						true
					);

					return !isoname.empty();
				}

				std::shared_ptr<Reinstall::Writer> WriterFactory() override {
					info() << "Saving '" << filename << "'" << endl;
					return Reinstall::Writer::FileFactory(isoname.c_str());
				}

			};

			Reinstall::push_back(node,make_shared<Action>(node));

			return true;
		}

	};

	return new Module();

 }

