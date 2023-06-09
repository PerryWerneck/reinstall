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

 #define LIBISOFS_WITHOUT_LIBBURN
 #include <libisofs/libisofs.h>

 typedef struct Iso_Image IsoImage;
 typedef struct iso_write_opts IsoWriteOpts;

 using namespace Udjat;
 using namespace std;
 using namespace Reinstall;

 namespace iso9660 {

	class UDJAT_PRIVATE IsoBuilderSingleTon {
	private:
		IsoBuilderSingleTon() {
			Logger::String{"Starting isofs"}.info("iso9660");
			iso_init();
		}

	public:
		static IsoBuilderSingleTon &getInstance() {
			static IsoBuilderSingleTon instance;
			return instance;
		}

		~IsoBuilderSingleTon() {
			Logger::String{"Stopping isofs"}.info("iso9660");
			iso_finish();
		}

	};

	static IsoDir * getIsoDir(IsoImage *image, const char *dirname) {

		if(*dirname == '/') {
			dirname++;
		}

		int rc;
		IsoDir * dir = iso_image_get_root(image);

		for(auto dn : Udjat::String(dirname).split("/")) {

			IsoNode *node;
			rc = iso_image_dir_get_node(image,dir,dn.c_str(),&node,0);
			if(rc == 0) {

				// Not found, add it.
				rc = iso_tree_add_new_dir(dir, dn.c_str(), (IsoDir **) &node);

			};

			if(rc < 0) {
				Logger::String{"Error '",iso_error_to_msg(rc),"' adding path ",dirname}.error("iso9660");
				throw runtime_error(iso_error_to_msg(rc));
			}

			dir = (IsoDir *) node;

		}

		return dir;

	}

	std::shared_ptr<Reinstall::Builder> BuilderFactory(const Settings &settings) {

		class Builder : public Reinstall::Builder {
		private:
			const iso9660::Settings &settings;

			class TempFile : public string {
			public:
				TempFile() : string{Udjat::File::Temporary::create()} {
				}

				~TempFile() {
					if(unlink(c_str()) < 0) {
						Logger::String{"Cant remove '",c_str(),"': ",strerror(errno)}.warning("iso9660");
					}
				}

			};

			std::vector<TempFile> tempfiles;

			IsoImage *image = nullptr;
			IsoWriteOpts *opts;

		public:
			Builder(const iso9660::Settings &s) : Reinstall::Builder("iso9660"), settings{s} {

				IsoBuilderSingleTon::getInstance();

				if(!iso_image_new("name", &image)) {
					throw runtime_error(_("Error creating iso image"));
				}

				iso_image_attach_data(image,this,NULL);

				iso_write_opts_new(&opts, 2);
				iso_write_opts_set_relaxed_vol_atts(opts, 1);
				iso_write_opts_set_rrip_version_1_10(opts,1);

			}

			virtual ~Builder() {

				iso_image_unref(image);
				iso_write_opts_free(opts);

			}

			void pre() override {
			}

			void push_back(std::shared_ptr<Reinstall::Source::File> file) override {

				string filename;

				if(file->remote()) {

					// Is a remote file, download it.

					tempfiles.emplace_back();
					filename = tempfiles.back();

					debug("Writing ",filename);
					Dialog::Progress &progress = Dialog::Progress::getInstance();

					File::Handler fil{filename.c_str(),true};

					file->save([&fil,&progress](unsigned long long current, unsigned long long total, const void *buf, size_t length){
						progress.set_progress(current,total);
						fil.write(current, buf, length);
					});

				} else {

					filename = file->path();

				}

				//
				// Add 'filename' in the iso image
				//
				int rc = 0;

				const char *isoname = file->c_str();
				auto pos = strrchr(isoname,'/');
				if(pos) {

					if(!*(pos+1)) {
						Logger::String{
							"Can't insert node '",
							isoname,
							"' it's not a FILE name, looks like a DIRECTORY name"
						}.error("iso9660");
						throw logic_error(Logger::Message{_("Unexpected or invalid file name: {}"),isoname});
					}

					// Add file to iso.
					rc = iso_tree_add_new_node(
						image,
						getIsoDir(image,string{isoname,(size_t) (pos - isoname)}.c_str()),
						pos+1,
						filename.c_str(),
						NULL
					);

				} else {

					// No path, store on root.
					rc = iso_tree_add_new_node(
						image,
						iso_image_get_root(image),
						isoname,
						filename.c_str(),
						NULL
					);

				}

				if(rc < 0) {
					Logger::String{
						"Error '",
						iso_error_to_msg(rc),
						"' adding '",
						isoname
					}.error("iso9660");
					throw runtime_error(iso_error_to_msg(rc));
				}

			}

			void post() override {
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

				const IsoBuilder *action = dynamic_cast<const IsoBuilder *>(&ptr);

				if(!action) {
					throw runtime_error(_("Rejecting invalid action pointer"));
				}

				Reinstall::Dialog::Progress::getInstance().set_sub_title(_("Setting up ISO image"));

				set_rockridge();
				set_joliet();
				set_allow_deep_paths();

				if(action->boot.eltorito) {

					Reinstall::Dialog::Progress::getInstance().set_sub_title(_("Adding el-torito boot image"));

					// Search to confirm presence of the boot_image.
					const char *filename = action->source(action->boot.eltorito.image)->filename();
					if(!(filename && *filename)) {
						throw runtime_error(_("Unexpected filename on el-torito boot image"));
					}

					set_el_torito_boot_image(
						action->boot.eltorito.image,
						action->boot.catalog,
						action->volume_id
					);

					cout << "iso9660\tEl-torito boot image set to '" << action->boot.eltorito.image << "'" << endl;
				}

				if(action->boot.efi->enabled()) {

					Reinstall::Dialog::Progress::getInstance().set_sub_title(_("Adding EFI boot image"));

					auto source = action->source(action->boot.efi->path());
					const char *filename = source->filename(true);
					if(!filename[0]) {
						throw runtime_error(_("Unexpected filename on EFI boot image"));
					}

					// Apply templates on EFI boot image.
					{
						debug("Applying templates on EFI boot image at '",filename,"'");

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
					Logger::String{"Adding ",filename," as EFI boot image"}.info(name);
					set_efi_boot_image(filename);

					if(action->boot.catalog && *action->boot.catalog) {
						Logger::String{"Adding ",source->path," as boot image"}.info(name);
						add_boot_image(source->path,0xEF);
					} else {
						Logger::String{"No boot catalog, ",source->path," was not added as boot image"}.trace(name);
					}

				}

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

