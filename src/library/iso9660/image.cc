/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <libreinstall/iso9660.h>
 #include <libisofs/libisofs.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <stdexcept>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace Udjat;
 using namespace std;

 namespace iso9660 {

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

	Image::Image(const char *name) {

		class IsoBuilderSingleTon {
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


		IsoBuilderSingleTon::getInstance();

		if(name && *name) {

			Logger::String{"Building image '",name,"'"}.trace("iso9660");
			if(!iso_image_new(name, &image)) {
				throw runtime_error(_("Error creating iso image"));
			}

		} else {

			if(!iso_image_new(Config::Value<string>{"iso9660","image-name",PACKAGE_NAME}.c_str(), &image)) {
				throw runtime_error(_("Error creating iso image"));
			}

		}

		iso_image_attach_data(image,this,NULL);

		iso_write_opts_new(&opts, 2);
		iso_write_opts_set_relaxed_vol_atts(opts, 1);
		iso_write_opts_set_rrip_version_1_10(opts,1);

	}

	Image::~Image() {
		iso_image_unref(image);
		iso_write_opts_free(opts);
	}

	void Image::add(const char *filename, const char *isoname) {

		Logger::String{filename," -> ", isoname}.write(Logger::Debug,"iso9660");

		int rc = 0;

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
				filename,
				NULL
			);

		} else {

			// No path, store on root.
			rc = iso_tree_add_new_node(
				image,
				iso_image_get_root(image),
				isoname,
				filename,
				NULL
			);

		}

		if(rc < 0) {
			Logger::String{
				"Error '",
				iso_error_to_msg(rc),
				"' adding '",
				isoname,
				"'"
			}.error("iso9660");
			throw runtime_error(iso_error_to_msg(rc));
		}


	}

	void Image::set_system_area(const char *path) {

		char data[32768];
		memset(data,0,sizeof(data));

		int fd;

		if(path && *path) {
			fd = open(path,O_RDONLY);
		} else {
			fd = open(Config::Value<string>("iso9660","system-area","/usr/share/syslinux/isohdpfx.bin").c_str(),O_RDONLY);
		}

		if(fd <  0) {
			throw system_error(errno, system_category(), _("Error loading system area"));
		}

		try {

			if(::read(fd,data,sizeof(data)) < 1) {
				throw system_error(errno, system_category(), _("Cant read system area definition file"));
			}

			int rc = iso_write_opts_set_system_area(opts,data,2,0);
			if(rc != ISO_SUCCESS) {
				throw runtime_error(iso_error_to_msg(rc));
			}

		} catch(...) {
			::close(fd);
			throw;
		}

		::close(fd);

	}

	void Image::set_volume_id(const char *volume_id) {

		if(volume_id && *volume_id) {
			iso_image_set_volume_id(image, volume_id);
		}

	}

	void Image::set_publisher_id(const char *publisher_id) {

		if(publisher_id && *publisher_id) {
			iso_image_set_publisher_id(image, publisher_id);
		} else {
			iso_image_set_publisher_id(image, Config::Value<string>("iso9660","publisher-id",PACKAGE_STRING).c_str());
		}

	}

	void Image::set_data_preparer_id(const char *data_preparer_id) {

		if(data_preparer_id && *data_preparer_id) {

			iso_image_set_data_preparer_id(image, data_preparer_id);

		} else {

			char username[32];
			if(getlogin_r(username, 32) == 0) {
				iso_image_set_data_preparer_id(image, username);
			}

		}

	}

	void Image::set_application_id(const char *application_id) {

		if(application_id && *application_id) {

			iso_image_set_application_id(image,application_id);

		} else {

			iso_image_set_application_id(image,Config::Value<string>("iso9660","application-id",PACKAGE_NAME).c_str());

		}

	}

	void Image::set_system_id(const char *system_id) {

		if(system_id && *system_id) {

			iso_image_set_system_id(image,system_id);

		} else {

			iso_image_set_system_id(image,Config::Value<string>("iso9660","system-id","LINUX").c_str());;

		}

	}

	void Image::set_iso_level(int level) {
		iso_write_opts_set_iso_level(opts, level);
	}

	void Image::set_rockridge(int rockridge) {
		iso_write_opts_set_rockridge(opts, rockridge);
	}

	void Image::set_joliet(int joliet) {
		iso_write_opts_set_joliet(opts, joliet);
	}

	void Image::set_allow_deep_paths(int deep_paths) {
		iso_write_opts_set_allow_deep_paths(opts, deep_paths);
	}

	void Image::set_part_like_isohybrid() {
		iso_write_opts_set_part_like_isohybrid(opts, 1);
	}

 }

 /*
 #include <iostream>
 #include <reinstall/iso9660.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/string.h>
 #include <reinstall/dialogs.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/url.h>

 #include <sys/stat.h>
 #include <fcntl.h>

 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 #define LIBISOFS_WITHOUT_LIBBURN
 #include <libisofs/libisofs.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {


	bool Image::apply(Source &source) {

		if(!Reinstall::Builder::apply(source)) {
			return false;
		}

		// Download and save to temporary file.
		if(strncasecmp(source.url,"file://",7)) {

			// It's not file, download it.
			source.save();

		} else {

			// file:// scheme, is it pointing to an existant file?

			Udjat::URL u{source.url};
			if(u.test() == 200) {

				// file:// scheme pointing to an existent file, use it.
				source.set_filename(u.ComponentsFactory().path.c_str());

			} else {

				// The response for 'test()' in a local file should be 200, but, if not try to download it anyway
				source.warning() << "Test for '" << u << "' failed, downloading" << endl;
				source.save();
			}

		}

		int rc = 0;

		auto pos = strrchr(source.path,'/');
		if(pos) {

			if(!*(pos+1)) {
				cerr << "iso9660\tCan't insert node '" << source.path << "' it's not a FILE name, looks like a DIRECTORY name" << endl;
				throw logic_error(_("Unexpanded path in source list"));
			}

			// Has path, get iso dir.
			rc = iso_tree_add_new_node(
				image,
				getIsoDir(image,string(source.path,pos - source.path).c_str()),
				pos+1,
				source.filename(),
				NULL
			);

		} else {

			// No path, store on root.
			rc = iso_tree_add_new_node(
				image,
				iso_image_get_root(image),
				source.path,
				source.filename(),
				NULL
			);

		}

		if(rc < 0) {
			cerr << "iso9660\tError '" << iso_error_to_msg(rc) << "' adding node" << endl;
			throw runtime_error(iso_error_to_msg(rc));
		}

		return true;

	}

	void Image::set_el_torito_boot_image(const char *isopath, const char *catalog, const char *id) {


	}

	void Image::add_boot_image(const char *isopath, uint8_t id) {
		ElToritoBootImage *bootimg = NULL;
		int rc = iso_image_add_boot_image(image,isopath,ELTORITO_NO_EMUL,0,&bootimg);
		if(rc < 0) {
			cerr << "iso9660\tError '" << iso_error_to_msg(rc) << "' adding boot image" << endl;
			throw runtime_error(iso_error_to_msg(rc));
		}

		el_torito_set_boot_platform_id(bootimg, id);
	}

	void Image::set_efi_boot_image(const char *boot_image, bool like_iso_hybrid) {

		if(like_iso_hybrid) {

			set_part_like_isohybrid();

			// Isohybrid, set partition
			int rc = iso_write_opts_set_partition_img(opts,2,0xef,(char *) boot_image,0);
			if(rc != ISO_SUCCESS) {
				cerr << "iso9660\tError '" << iso_error_to_msg(rc) << "' setting EFI partition" << endl;
				throw runtime_error(iso_error_to_msg(rc));
			}

			cout << "iso9660\tEFI partition set from '" << boot_image << "'" << endl;

		} else {

			// Not isohybrid.
			int rc = iso_write_opts_set_efi_bootp(opts,(char *) boot_image,0);
			if(rc != ISO_SUCCESS) {
				cerr << "iso9660\tError '" << iso_error_to_msg(rc) << "' setting EFI boot image" << endl;
				throw runtime_error(iso_error_to_msg(rc));
			}

			cout << "iso9660\tEFI bootp set from '" << boot_image << "'" << endl;

		}

	}

	size_t Image::size() {

		rc = iso_image_update_sizes(image);
		if (rc < 0) {
			cerr << "iso9660\tError '" << iso_error_to_msg(rc) << "' in iso_image_update_sizes()" << endl;
			throw runtime_error(iso_error_to_msg(rc));
		}

	}

	std::shared_ptr<Writer> Image::burn(std::shared_ptr<Writer> writer) {

		debug("Burning ISO image");

		int rc;

		Dialog::Progress &progress = Dialog::Progress::getInstance();
		progress.set_sub_title(_("Preparing to write"));

		rc = iso_image_update_sizes(image);
		if (rc < 0) {
			cerr << "iso9660\tError '" << iso_error_to_msg(rc) << "' in iso_image_update_sizes()" << endl;
			throw runtime_error(iso_error_to_msg(rc));
		}

		struct burn_source *burn_src = NULL;

		rc = iso_image_create_burn_source(image, opts, &burn_src);
		if (rc < 0) {
			cerr << "iso9660\tError '" << iso_error_to_msg(rc) << "' in iso_image_create_burn_source()" << endl;
			throw runtime_error(iso_error_to_msg(rc));
		}

		writer->open();

		progress.set_sub_title(_("Writing image"));
		try {

			#define BUFLEN 2048
			unsigned char buffer[BUFLEN];

			double current = 0;
			double total = burn_src->get_size(burn_src);

			while(burn_src->read_xt(burn_src, buffer, BUFLEN) == BUFLEN) {

				writer->write(buffer,BUFLEN);

				current += BUFLEN;
				if(total) {
					progress.set_progress(current,total);
				}

			}


		} catch(...) {

			burn_src->free_data(burn_src);
			throw;

		}

		burn_src->free_data(burn_src);
		free(burn_src);

		progress.set_sub_title(_("Finalizing"));
		writer->finalize();
		writer->close();

		progress.set_sub_title("");

		return writer;
	}

 }

 */
