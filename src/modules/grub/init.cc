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
 #include <reinstall/builder.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <reinstall/source.h>
 #include <reinstall/sources/kernel.h>
 #include <reinstall/sources/initrd.h>
 #include <reinstall/action.h>

 using namespace std;
 using namespace Udjat;

 Udjat::Module * udjat_module_init() {

 	static const Udjat::ModuleInfo moduleinfo { "Grub based system installation" };

	class Module : public Udjat::Module, public Udjat::Factory {
	public:
		Module() : Udjat::Module("local-installer", moduleinfo), Udjat::Factory("local-installer",moduleinfo) {
		}

		bool generic(const pugi::xml_node &node) override {

			class Action : public Reinstall::Action {
			private:
				std::shared_ptr<Reinstall::Kernel> kernel;
				std::shared_ptr<Reinstall::InitRD> initrd;

			public:
				Action(const pugi::xml_node &node) : Reinstall::Action(node,"computer") {

					// Get URL for installation kernel.
					if(!scan(node,"kernel",[this](const pugi::xml_node &node) {
						kernel = make_shared<Reinstall::Kernel>(node);
						push_back(kernel);
						return true;
					})) {
						throw runtime_error(_("Missing required entry <kernel> with the URL for installation kernel"));
					}

					// Get URL for installation init.
					if(!scan(node,"init",[this](const pugi::xml_node &node) {
						initrd = make_shared<Reinstall::InitRD>(node);
						push_back(initrd);
						return true;
					})) {
						throw runtime_error(_("Missing required entry <init> with the URL for the linuxrc program"));
					}

					debug("sources.size=",sources.size());

				}

				std::shared_ptr<Reinstall::Builder> BuilderFactory() override {

					class Builder : public Reinstall::Builder {
					public:
						Builder() = default;

						void pre(const Reinstall::Action &action) override {
						}

						bool apply(Reinstall::Source &source) override {

							if(!Reinstall::Builder::apply(source)) {
								return false;
							}

							source.save(source.path);

							return true;

						}

						void build(Reinstall::Action &action) override {
						}

						void post(const Reinstall::Action &action) override {
						}

						std::shared_ptr<Reinstall::Writer> burn(std::shared_ptr<Reinstall::Writer> writer) override {

							return writer;
						}


					};

					return(make_shared<Builder>());

				}

				std::shared_ptr<Reinstall::Writer> WriterFactory() override {
					return make_shared<Reinstall::Writer>(*this);
				}

				std::string getProperty(const char *key) const {
					std::string value;
					if(getProperty(key,value)) {
						return value;
					}
					throw runtime_error(Logger::Message{_("Unable to get value for '{}'"),key});
				}

				bool getProperty(const char *key, std::string &value) const override {

					debug("Searching for '",key,"' in ",name());

					if(strcasecmp(key,"kernel-fspath") == 0 || strcasecmp(key,"kernel-rpath") == 0 ) {
						value = (kernel->rpath()+1);
						return true;
					}

					if(strcasecmp(key,"initrd-fspath") == 0 || strcasecmp(key,"initrd-rpath") == 0 ) {
						value = (initrd->rpath()+1);
						return true;
					}

					if(strcasecmp(key,"boot-dir") == 0 || strcasecmp(key,"grub-path") == 0) {

						// TODO: Detect boot path ( /proc/cmdline? )
#ifdef DEBUG
						value = "/tmp";
#else
						value = "/boot";
#endif // DEBUG
						return true;
					}

					if(strcasecmp(key,"grub-conf-dir") == 0) {
#ifdef DEBUG
						value = "/tmp";
#else
						value = "/etc/grub.d";
#endif // DEBUG
						return true;
					}

					if(strcasecmp(key,"install-enabled") == 0) {
						value = "1";
						return true;
					}

					/*
					if(strcasecmp(key,"kernel-path") == 0) {
						// TODO: Detect boot partition
						value = "/boot/kernel-" PACKAGE_NAME;
						debug(key,"=",value);
						return true;
					}

					if(strcasecmp(key,"initrd-path") == 0) {
						// TODO: Detect boot partition
						value = "/boot/initrd-" PACKAGE_NAME;
						debug(key,"=",value);
						return true;
					}
					*/

					if(strcasecmp(key,"kernel-filename") == 0) {
						value = "kernel." PACKAGE_NAME;
						return true;
					}

					if(strcasecmp(key,"initrd-filename") == 0) {
						value = "initrd." PACKAGE_NAME;
						return true;
					}

					if(strcasecmp(key,"kernel-file") == 0) {
						value = getProperty("grub-path") + "/" + getProperty("kernel-filename");
						return true;
					}

					if(strcasecmp(key,"initrd-file") == 0) {
						value = getProperty("grub-path") + "/" + getProperty("initrd-filename");
						return true;
					}

					if(Reinstall::Action::getProperty(key,value)) {
						return true;
					}

					throw runtime_error(Logger::Message{_("Unable to get value for '{}'"),key});

				}

			};

			Reinstall::push_back(node,make_shared<Action>(node));

			return true;
		}

	};

	return new Module();

 }

