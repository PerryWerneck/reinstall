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
 #include <reinstall/actions/isobuilder.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/string.h>
 #include <iostream>
 #include <reinstall/dialogs.h>
 #include <reinstall/diskimage.h>
 #include <cstdlib>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <reinstall/userinterface.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	IsoBuilder::IsoBuilder(const pugi::xml_node &node, const char *icon_name)
		: Reinstall::Action(node,icon_name) {

		// Get options.
		system_area = getAttribute(
							node,
							"iso-9660",
							"system-area",
							system_area
						);

		volume_id =  getAttribute(
							node,
							"iso-9660",
							"volume-id",
							volume_id
						);

		publisher_id =  getAttribute(
							node,
							"iso-9660",
							"publisher-id",
							publisher_id
						);

		data_preparer_id =  getAttribute(
							node,
							"iso-9660",
							"data-preparer-id",
							data_preparer_id
						);

		application_id =  getAttribute(
							node,
							"iso-9660",
							"application-id",
							application_id
						);

		system_id =  getAttribute(
							node,
							"iso-9660",
							"system-id",
							system_id
						);


		eltorito.enabled = getAttribute(
							node,
							"iso-9660",
							"eltorito",
							eltorito.enabled
						);

		eltorito.boot_image = getAttribute(
							node,
							"iso-9660",
							"eltorito-boot-image",
							eltorito.boot_image
						);

		eltorito.catalog =  getAttribute(
							node,
							"iso-9660",
							"eltorito-boot-catalog",
							eltorito.catalog
						);

		{
			auto bootnode = node.child("efi-boot-image");
			if(bootnode) {
				Logger::String{"Using customized EFI Boot image"}.trace(name());
				efibootimage = EFIBootImage::factory(bootnode);
			} else {
				Logger::String{"Using default EFI Boot image"}.trace(name());
				efibootimage = make_shared<EFIBootImage>(node);
			}
		}


	}

	IsoBuilder::~IsoBuilder() {
	}

	std::shared_ptr<Reinstall::Builder> IsoBuilder::BuilderFactory() {

		class Builder : public Reinstall::iso9660::Builder {
		public:
			Builder() {
			}

			void pre(const Action &ptr) override {

				const IsoBuilder *action = dynamic_cast<const IsoBuilder *>(&ptr);

				if(!action) {
					throw runtime_error(_("Rejecting invalid action pointer"));
				}

				Reinstall::Dialog::Progress::getInstance().set_sub_title(_("Setting up ISO image"));

				set_system_area(action->system_area);
				set_volume_id(action->volume_id);
				set_publisher_id(action->publisher_id);
				set_data_preparer_id(action->data_preparer_id);
				set_system_id(action->system_id);
				set_application_id(action->application_id);
			}

			void build(const Action &ptr) override {
			}

			void post(const Action &ptr) override {

				const IsoBuilder *action = dynamic_cast<const IsoBuilder *>(&ptr);

				if(!action) {
					throw runtime_error(_("Rejecting invalid action pointer"));
				}

				Reinstall::Dialog::Progress::getInstance().set_sub_title(_("Setting up ISO image"));

				set_rockridge();
				set_joliet();
				set_allow_deep_paths();

				if(action->eltorito.enabled) {

					Reinstall::Dialog::Progress::getInstance().set_sub_title(_("Adding el-torito boot image"));

					// Search to confirm presence of the boot_image.
					const char *filename = action->source(action->eltorito.boot_image)->filename();
					if(!(filename && *filename)) {
						throw runtime_error(_("Unexpected filename on el-torito boot image"));
					}

					set_el_torito_boot_image(
						action->eltorito.boot_image,
						action->eltorito.catalog,
						action->volume_id
					);

					cout << "iso9660\tEl-torito boot image set to '" << action->eltorito.boot_image << "'" << endl;
				}

				if(action->efibootimage->enabled()) {

					Reinstall::Dialog::Progress::getInstance().set_sub_title(_("Adding EFI boot image"));

					auto source = action->source(action->efibootimage->path());
					const char *filename = source->filename();
					if(!filename[0]) {
						throw runtime_error(_("Unexpected filename on EFI boot image"));
					}

					// Apply templates on EFI boot image.
					{
						Disk::Image disk(filename);

						for(auto tmpl : action->templates) {

							disk.forEach([this,&tmpl](const char *mountpoint, const char *path){

								if(tmpl->test(path)) {
									tmpl->load((Udjat::Object &) *this);
									cout << "efi\tReplacing " << path << " with template " << tmpl->c_str() << endl;
									tmpl->replace((string{mountpoint} + "/" + path).c_str());
								}

							});

						}
					}

					// Add EFI boot image
					cout << "isobuilder\tAdding " << filename << " as EFI boot image" << endl;
					set_efi_boot_image(filename);

					cout << "isobuilder\tAdding " << source->path << " as boot image" << endl;
					add_boot_image(source->path,0xEF);

				}

			}

		};

		if(efibootimage->enabled()) {
			efibootimage->build(*this);
		}

		return make_shared<Builder>();

	}

	std::shared_ptr<Reinstall::Writer> IsoBuilder::WriterFactory() {
		return Reinstall::Writer::USBWriterFactory(*this);
	}

	bool IsoBuilder::interact() {

		return true;

	}

 }

