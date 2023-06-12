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
 #include <udjat/defs.h>
 #include <udjat/tools/object.h>
 #include <libreinstall/source.h>
 #include <libreinstall/iso9660.h>
 #include <libreinstall/builders/iso9660.h>
 #include <reinstall/dialogs/progress.h>
 #include <udjat/tools/file/temporary.h>
 #include <udjat/tools/file/handler.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/string.h>
 #include <vector>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 typedef struct Iso_Image IsoImage;
 typedef struct iso_write_opts IsoWriteOpts;

 using namespace Udjat;
 using namespace std;
 using namespace Reinstall;

 namespace iso9660 {

	std::shared_ptr<Reinstall::Builder> BuilderFactory(const Settings &settings) {

		class Builder : public Reinstall::Builder, private Image {
		private:
			const iso9660::Settings &settings;

			class TempFile : public string {
			public:
				TempFile(const TempFile &) = delete;
				TempFile(const TempFile *) = delete;

				TempFile() : string{Udjat::File::Temporary::create()} {
				}

				~TempFile() {
					if(unlink(c_str()) < 0) {
						Logger::String{"Cant remove '",c_str(),"': ",strerror(errno)}.warning("iso9660");
					}
				}

			};

			/// @brief Path for EFI boot partition in local filesystem;
			std::string efibootpart;

			std::vector<shared_ptr<TempFile>> tempfiles;

		public:
			Builder(const iso9660::Settings &s) : Reinstall::Builder("iso9660"), Image{s.name}, settings{s} {
			}

			virtual ~Builder() {
			}

			void pre() override {

				Reinstall::Dialog::Progress::getInstance().set_sub_title(_("Setting up ISO image"));

				set_system_area(settings.system_area);
				set_volume_id(settings.volume_id);
				set_publisher_id(settings.publisher_id);
				set_data_preparer_id(settings.data_preparer_id);
				set_system_id(settings.system_id);
				set_application_id(settings.application_id);

			}

			void push_back(std::shared_ptr<Reinstall::Source::File> file) override {

				/// @brief The system filename.
				string path;

				if(file->remote()) {

					// Is a remote file, download it.
					auto filename = make_shared<TempFile>();
					tempfiles.emplace_back(filename);

					debug("Writing ",filename->c_str());
					Dialog::Progress &progress = Dialog::Progress::getInstance();

					File::Handler fil{filename->c_str(),true};

					file->save([&fil,&progress](unsigned long long current, unsigned long long total, const void *buf, size_t length){
						progress.set_progress(current,total);
						fil.write(current, buf, length);
					});

					path = filename->c_str();

				} else {

					path = file->path();

				}

				add(path.c_str(),file->c_str());

				if(!strcmp(file->c_str(),settings.boot.efi.image)) {
					efibootpart = path;
				}

			}

			void post() override {

				Reinstall::Dialog::Progress &progress{Reinstall::Dialog::Progress::getInstance()};

				progress.set_sub_title(_("Setting up ISO image"));

				set_rockridge();
				set_joliet();
				set_allow_deep_paths();

				if(settings.boot.eltorito) {
					progress.set_sub_title(_("Installing el-torito boot image"));
					set_bootable(settings.boot.catalog,settings.boot.eltorito);
				}

				if(settings.boot.efi) {
					progress.set_sub_title(_("Installing efi boot image"));
					set_bootable(efibootpart.c_str(),settings.boot.efi);
				}

			}

			void write(std::shared_ptr<Writer> writer) override {
				Image::write(writer);
			}

		};

		return make_shared<Builder>(settings);

	}

 }

 /*
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


		boot.eltorito.enabled =
			getAttribute(
				node,
				"iso-9660",
				"eltorito",
				boot.eltorito.enabled
			);

		boot.eltorito.image =
			getAttribute(
				node,
				"iso-9660",
				"eltorito-boot-image",
				boot.eltorito.image
			);

		boot.catalog =
			getAttribute(
				node,
				"iso-9660",
				"boot-catalog",
				(boot.eltorito.enabled ? boot.catalog : "")
			);

		{
			auto bootnode = node.child("efi-boot-image");
			if(bootnode) {
				Logger::String{"Using customized EFI Boot image"}.trace(name());
				boot.efi = EFIBootImage::factory(bootnode);
			} else {
				Logger::String{"Using default EFI Boot image"}.trace(name());
				boot.efi = make_shared<EFIBootImage>(node);
			}
		}


	}

	IsoBuilder::~IsoBuilder() {
	}

	std::shared_ptr<Reinstall::Builder> IsoBuilder::BuilderFactory() {

		class Builder : public Reinstall::iso9660::Builder {
		private:
			std::shared_ptr<EFIBootImage> efibootimage;

		public:
			Builder(std::shared_ptr<EFIBootImage> e) : efibootimage{e} {
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

			void build(Action &action) override {

				if(efibootimage->enabled()) {
					efibootimage->build(action);
				}
			}

			void post(const Action &ptr) override {


			}

		};

		return make_shared<Builder>(boot.efi);

	}

	std::shared_ptr<Reinstall::Writer> IsoBuilder::WriterFactory() {
		return Reinstall::Writer::USBWriterFactory(*this);
	}

	bool IsoBuilder::interact() {

		return true;

	}

 }
 */

