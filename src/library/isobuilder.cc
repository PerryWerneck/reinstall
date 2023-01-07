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
 #include <reinstall/sources/kernel.h>
 #include <reinstall/sources/initrd.h>
 #include <reinstall/actions/isobuilder.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/string.h>
 #include <iostream>
 #include <reinstall/dialogs.h>
 #include <reinstall/diskimage.h>
 #include <cstdlib>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/intl.h>

 using namespace std;

 namespace Reinstall {

	IsoBuilder::IsoBuilder(const pugi::xml_node &node, const char *icon_name)
		: Reinstall::Action(node,icon_name), filename{getAttribute(node,"output-file","")} {

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

		efi.boot_image = getAttribute(
							node,
							"iso-9660",
							"efi-boot-image",
							efi.boot_image
						);


	}

	IsoBuilder::~IsoBuilder() {
	}

	/*
	void IsoBuilder::post(const char *image) {

		cout << "isobuilder\tRunning post scripts" << endl;
		for(Script &script : post_scripts) {

			Udjat::String cmdline{script.cmdline()};

			cmdline.expand(*this);

			cmdline.expand([image](const char *key, std::string &value){
				if(!strcasecmp(key,"devname")) {
					value = image;
					return true;
				}
				return false;
			});

			cout << "isobuilder\tRunning '" << cmdline << "'" << endl;

			if(Udjat::SubProcess(cmdline.c_str()).run() != 0) {
				throw runtime_error(_("Error on post-script"));
			}

			cout << "isobuilder\tScript '" << cmdline << "' is complete" << endl;

		}

		*/ /*
		if(efi.isohibrid_cmdline && *efi.isohibrid_cmdline) {
			// Apply isohybrid
			cout << "isobuilder\tPatching image" << endl;

			Udjat::String cmdline{efi.isohibrid_cmdline};

			cmdline.expand(*this);

			cmdline.expand([image](const char *key, std::string &value){
				if(!strcasecmp(key,"isoname")) {
					value = image;
					return true;
				}
				return false;
			});

			if(system(cmdline.c_str()) != 0) {
				throw runtime_error("Error patching image");
			}

			cout << "isobuilder\tCommand '" << cmdline << "' runs without error" << endl;

		}
		*/ /*

	}
	*/

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

				if(action->eltorito.boot_image && *action->eltorito.boot_image) {

					Reinstall::Dialog::Progress::getInstance().set_sub_title(_("Adding el-torito boot image"));

					// Search to confirm presence of the boot_image.
					const char *filename = action->source(action->eltorito.boot_image)->filename;
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

				if(action->efi.boot_image && *action->efi.boot_image) {

					Reinstall::Dialog::Progress::getInstance().set_sub_title(_("Adding EFI boot image"));

					auto source = action->source(action->efi.boot_image);
					if(!(source->filename && *source->filename)) {
						throw runtime_error(_("Unexpected filename on EFI boot image"));
					}

					// Apply templates on EFI boot image.
					{
						Disk::Image disk(source->filename);

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
					set_efi_boot_image(source->filename);

					cout << "isobuilder\tAdding " << source->path << " as boot image" << endl;
					add_boot_image(source->path,0xEF);


				}

			}

		};

		return make_shared<Builder>();

		/*
		Reinstall::Dialog::Progress &progress = Reinstall::Dialog::Progress::getInstance();

		// Create and activate worker
		auto worker = make_shared<Reinstall::iso9660::Builder>();

		progress.set_sub_title(_("Building ISO image"));

		// Setup isofs.
		worker->set_system_area(system_area);
		worker->set_volume_id(volume_id);
		worker->set_publisher_id(publisher_id);
		worker->set_data_preparer_id(data_preparer_id);
		worker->set_system_id(system_id);
		worker->set_application_id(application_id);

		Reinstall::Action::prepare(*worker);

		// Finalize
		//worker.set_iso_level();
		worker->set_rockridge();
		worker->set_joliet();
		worker->set_allow_deep_paths();

		if(eltorito.boot_image && *eltorito.boot_image) {

			// Search to confirm presence of the boot_image.
			const char *filename = source(eltorito.boot_image)->filename;
			if(!(filename && *filename)) {
				throw runtime_error(_("Unexpected filename on el-torito boot image"));
			}

			worker->set_el_torito_boot_image(
				eltorito.boot_image,
				eltorito.catalog,
				volume_id
			);

			cout << "iso9660\tEl-torito boot image set to '" << eltorito.boot_image << "'" << endl;
		}

		if(efi.boot_image && *efi.boot_image) {

			auto source = this->source(efi.boot_image);
			if(!(source->filename && *source->filename)) {
				throw runtime_error(_("Unexpected filename on EFI boot image"));
			}

			// Apply templates on EFI boot image.
			{
				Disk::Image disk(source->filename);

				for(auto tmpl : templates) {

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
			worker->set_efi_boot_image(source->filename);

			cout << "isobuilder\tAdding " << source->path << " as boot image" << endl;
			worker->add_boot_image(source->path,0xEF);
		}

		return worker;
		*/

	}

	std::shared_ptr<Reinstall::Writer> IsoBuilder::WriterFactory() {
		if(filename && *filename) {
			info() << "Saving image to '" << filename << "'" << endl;
			return Reinstall::Writer::FileWriterFactory(*this,filename);
		}
		info() << "Asking for USB Storage device" << endl;
		return Reinstall::Writer::USBWriterFactory(*this);
	}

 }

