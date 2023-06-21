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
 #include <libreinstall/template.h>
 #include <libreinstall/builders/iso9660.h>
 #include <udjat/ui/dialogs/progress.h>
 #include <udjat/tools/file/temporary.h>
 #include <udjat/tools/file/handler.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/string.h>
 #include <vector>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 // FatFS is required to manage the EFI Boot image.
 #include <ff.h>
 #include <diskio.h>

 typedef struct Iso_Image IsoImage;
 typedef struct iso_write_opts IsoWriteOpts;

 using namespace Udjat;
 using namespace std;
 using namespace Reinstall;

 using Progress = Udjat::Dialog::Progress;

 namespace iso9660 {

 	static void scandirs(const Udjat::Abstract::Object &object, const std::vector<Reinstall::Template> &tmpls, const char *path) {

		// https://github.com/maskedw/fatfs/blob/master/stm32/main.c
		DIR dirs;
		memset(&dirs,0,sizeof(dirs));

		FRESULT rc;

		rc = f_opendir(&dirs, path);
		if(rc != FR_OK) {
			Logger::String{"Unexpected error ",rc," opening fat fs dir '",path,"'"}.error("iso9660");
			throw runtime_error("Unexpected error patching EFI Boot image");
		}

		FILINFO Finfo;
		memset(&Finfo,0,sizeof(Finfo));

		while(((rc = f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0]) {

			if (Finfo.fname[0] == '.') {
				continue;
			}

			string filename{path};
			filename += Finfo.fname; // (*Finfo.lfname ? Finfo.lfname : Finfo.fname);

			Logger::String{"fat://",filename}.write(Logger::Debug,"efiboot");

			if (Finfo.fattrib & AM_DIR) {
				scandirs(object,tmpls,(filename+"/").c_str());
				continue;
			}

			for(const Reinstall::Template &tmpl : tmpls) {

				if(tmpl.test(filename.c_str())) {

					// Found template
					Logger::String{"Replacing file fat://",filename," with template"}.trace(tmpl.name());
					String text{tmpl.get()};
					text.expand(object,true,true);

					FIL fp;
					memset(&fp,0,sizeof(fp));

					rc = f_open(&fp, filename.c_str(), FA_OPEN_EXISTING | FA_WRITE);
					if(rc != FR_OK) {
						Logger::String{"Unexpected error ",rc," opening '",filename,"'"}.error("iso9660");
						throw runtime_error("Unexpected error patching EFI Boot image");
					}

					rc = f_truncate(&fp);
					if(rc != FR_OK) {
						Logger::String{"Unexpected error ",rc," truncating '",filename,"'"}.error("iso9660");
						throw runtime_error("Unexpected error patching EFI Boot image");
					}

					// Write 'text' on file.
					{
						const char *ptr = text.c_str();
						UINT length = (UINT) text.size();

						while(length) {

							UINT bytes = 0;
							rc = f_write(&fp,ptr,length,&bytes);
							if(rc != FR_OK) {
								Logger::String{"Unexpected error ",rc," writing '",filename,"'"}.error("iso9660");
								throw runtime_error("Unexpected error patching EFI Boot image");
							}

							debug("length=",length," wrote=",bytes);
							length -= bytes;
							ptr += bytes;

						}

					}

					rc = f_close(&fp);
					if(rc != FR_OK) {
						Logger::String{"Unexpected error ",rc," closing '",filename,"'"}.error("iso9660");
						throw runtime_error("Unexpected error patching EFI Boot image");
					}
					break;
				}
			}

		}
		debug("rc=",rc);

		if(rc != FR_OK) {
			Logger::String{"Unexpected error ",rc," reading fat fs dir '",path,"'"}.error("iso9660");
		}

 	}

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

			/// @brief Path for EFI boot partition in local filesystem.
			std::string efibootpart;

			std::vector<shared_ptr<TempFile>> tempfiles;

		public:
			Builder(const iso9660::Settings &s) : Reinstall::Builder("iso9660"), Image{s.name}, settings{s} {
			}

			virtual ~Builder() {
			}

			void pre() override {

				Progress::instance().message(_("Setting up ISO image"));

				set_system_area(settings.system_area);
				set_volume_id(settings.volume_id);
				set_publisher_id(settings.publisher_id);
				set_data_preparer_id(settings.data_preparer_id);
				set_system_id(settings.system_id);
				set_application_id(settings.application_id);

			}

			void push_back(const Udjat::Abstract::Object &object, const std::vector<Reinstall::Template> &tmpls) override {

				if(efibootpart.empty()) {
					return;
				}

				// Apply templates on efi boot image.
				Logger::String{"Applying templates on ",efibootpart}.trace("iso9660");

				int fd = ::open(efibootpart.c_str(),O_RDWR);
				if(fd < 0) {
					throw system_error(errno,system_category(),"Cant open EFI boot image");
				}

				try {

					FATFS fs;
					int rc;

					if(disk_ioctl(0, CTRL_FORMAT, &fd) != RES_OK) {
						throw runtime_error("Cant bind fatfs to efi boot disk image");
					}

					memset(&fs,0,sizeof(fd));

					rc = f_mount(&fs, "0:", 1);
					if(rc != FR_OK) {
						throw runtime_error(Logger::String{"Unexpected error '",rc,"' on f_mount"});
					}

					try {

						scandirs(object,tmpls,"/");

					} catch(...) {

						f_mount(NULL, "", 0);
						throw;

					}

					f_mount(NULL, "", 0);

				} catch(...) {

					::close(fd);
					throw;

				}

				::close(fd);


			}

			void push_back(std::shared_ptr<Reinstall::Source::File> file) override {

				/// @brief The system filename.
				string path;
				bool efibootimage = !strcmp(file->c_str(),settings.boot.efi.image);

				if(file->remote() || efibootimage) {

					// Is a remote file or the efibootimage, download it.
					auto filename = make_shared<TempFile>();
					tempfiles.emplace_back(filename);

					Logger::String{"Saving temporary for ",file->c_str()," on '",filename->c_str(),"'"}.write(Logger::Debug,"iso9660");
					Progress &progress{Progress::instance()};

					File::Handler fil{filename->c_str(),true};

					file->save([&fil,&progress](unsigned long long current, unsigned long long total, const void *buf, size_t length){

						progress.progress(current,total);

						/*
						if(current == 0 && total) {
							Logger::String{"Alocating ",String{}.set_byte(total)," for temporary file"}.trace("iso9660");
							fil.allocate(total);
						}
						*/

						fil.write(current, buf, length);
					});

					path = filename->c_str();

				} else {

					path = file->path();

				}

				add(path.c_str(),file->c_str());

				if(efibootimage) {
					efibootpart = path;
				}

			}

			void post() override {

				Progress &progress{Progress::instance()};

				progress.message(_("Setting up ISO image"));
				progress.pulse();

				set_rockridge();
				set_joliet();
				set_allow_deep_paths();

				if(settings.boot.eltorito) {
					progress.url(_("Installing el-torito boot image"));
					set_bootable(settings.boot.catalog,settings.boot.eltorito);
				}

				if(settings.boot.efi) {
					progress.url(_("Installing efi boot image"));
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

