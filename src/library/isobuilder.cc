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


 #include <reinstall/actions/kernel.h>
 #include <reinstall/actions/initrd.h>
 #include <reinstall/actions/isobuilder.h>
 #include <iostream>
 #include <reinstall/dialogs.h>

 using namespace std;

 namespace Reinstall {

	IsoBuilder::IsoBuilder(const pugi::xml_node &node) : Reinstall::Action(node) {

		// Get URL for installation kernel.
		if(!scan(node,"kernel",[this](const pugi::xml_node &node) {
			push_back(make_shared<Reinstall::Kernel>(node));
			return true;
		})) {
			throw runtime_error("Missing required entry <kernel> with the URL for installation kernel");
		}

		// Get URL for installation init.
		if(!scan(node,"init",[this](const pugi::xml_node &node) {
			push_back(make_shared<Reinstall::InitRD>(node));
			return true;
		})) {
			throw runtime_error("Missing required entry <init> with the URL for the linuxrc program");
		}

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

		efi.boot_image =  getAttribute(
							node,
							"iso-9660",
							"efi-boot-image",
							efi.boot_image
						);

	}

	IsoBuilder::~IsoBuilder() {
	}

	void IsoBuilder::activate() {

		cout << "***************************" << __FILE__ << "(" << __LINE__ << ")" << endl;
		Reinstall::Dialog::Progress progress(*this);

		// Create and activate worker
		Reinstall::iso9660::Worker worker;

		progress.set("Building installation image");

		// Setup isofs.
		worker.set_system_area(system_area);
		worker.set_volume_id(volume_id);
		worker.set_publisher_id(publisher_id);
		worker.set_data_preparer_id(data_preparer_id);
		worker.set_system_id(system_id);
		worker.set_application_id(application_id);

		Reinstall::Action::activate(worker);

		// Finalize
		//worker.set_iso_level();
		worker.set_rockridge();
		worker.set_joliet();
		worker.set_allow_deep_paths();

		if(eltorito.boot_image && *eltorito.boot_image) {

			// Search to confirm presence of the boot_image.
			const char *filename = find(eltorito.boot_image)->filename;
			if(!(filename && *filename)) {
				throw runtime_error("Unexpected filename on el-torito boot image");
			}

			worker.set_el_torito_boot_image(
				eltorito.boot_image,
				eltorito.catalog,
				volume_id
			);

			cout << "iso9660\tEl-torito boot image set to '" << eltorito.boot_image << "'" << endl;
		}

		if(efi.boot_image && *efi.boot_image) {

			auto source = find(efi.boot_image);
			if(!(source->filename && *source->filename)) {
				throw runtime_error("Unexpected filename on EFI boot image");
			}

			// Apply templates on EFI boot image.
#ifndef DEBUG
			#error Missing templates on EFI boot image.
#endif // DEBUG

			// Add EFI boot image
			worker.set_efi_boot_image(source->filename);
			worker.add_boot_image(source->path,0xEF);

		}

		/*

		if(!efi.boot_image.empty()) {

#ifdef DEBUG
			cout << "Imagem de boot EFI em " << efi.boot_image << endl;
#endif // DEBUG

			// Obtém caminho do arquivo temporário
			auto tempfile = find(efi.boot_image.c_str());

			if(!templates.empty()) {

				// Aplica templates na imagem EFI.
#ifdef DEBUG
				cout << "  Aplicando templates em " << efi.boot_image << endl;
#endif // DEBUG

				// Procura por templates dentro da imagem vfat
				Disk::Image(tempfile.c_str())
					.forEach([this](const std::string &mountpoint, const std::string &dirname, const char *basename) {

					for(auto t = templates.begin();t != templates.end(); t++) {
						if(t->apply(mountpoint.c_str(),dirname.c_str(),basename)) {
							cout << "    Template '" << t->isopath << "' aplicado com sucesso" << endl;
							break;
						}
					}

				});

			}

			// Aplica partição na imagem
			image.set_efi_boot_image(tempfile.c_str());
			image.add_boot_image(efi.boot_image.c_str(),0xEF);
		*/

		// Write image to destination.
		write(worker);

	}

 }

