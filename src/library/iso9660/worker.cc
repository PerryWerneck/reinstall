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

 #include <iostream>
 #include <reinstall/iso9660.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/string.h>
 #include <reinstall/dialogs.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/intl.h>

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

	class UDJAT_PRIVATE IsoBuilderSingleTon {
	private:
		IsoBuilderSingleTon() {
			cout << "iso9660\tStarting iso builder" << endl;
			iso_init();
		}

	public:
		static IsoBuilderSingleTon &getInstance() {
			static IsoBuilderSingleTon instance;
			return instance;
		}

		~IsoBuilderSingleTon() {
			iso_finish();
			cout << "iso9660\tIso builder was terminated" << endl;
		}

	};

	iso9660::Worker::Worker() {

		IsoBuilderSingleTon::getInstance();

		if(!iso_image_new("name", &image)) {
			throw runtime_error(_("Error creating iso image"));
		}

		iso_image_attach_data(image,this,NULL);

		iso_write_opts_new(&opts, 2);
		iso_write_opts_set_relaxed_vol_atts(opts, 1);
		iso_write_opts_set_rrip_version_1_10(opts,1);

	}

	iso9660::Worker::~Worker() {
		iso_image_unref(image);
		iso_write_opts_free(opts);
	}

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
				cerr << "iso9660\tError '" << iso_error_to_msg(rc) << "' adding path " << dirname << endl;
				throw runtime_error(iso_error_to_msg(rc));
			}

			dir = (IsoDir *) node;

		}

		return dir;

	}

	void iso9660::Worker::push_back(Source &source) {

		if(!(source.path && *source.path)) {
			// No local path, ignore it.
			return;
		}

		// Download and apply files.
		string filename;
		if(source.filename) {
			filename = source.filename;
		}

		cout << "iso9660\t" << source.url << " -> " << source.path << endl;

		if(filename.empty()) {

			// No local filename, download the file to get one.
			filename = source.save();

		}

		int rc = 0;

		auto pos = strrchr(source.path,'/');
		if(pos) {

			if(!*(pos+1)) {
				cerr << "iso9660\tCan't insert node '" << source.path << "' it's not a FILE name" << endl;
				throw logic_error(_("Unexpanded path in source list"));
			}

			// Has path, get iso dir.
			rc = iso_tree_add_new_node(
				image,
				getIsoDir(image,string(source.path,pos - source.path).c_str()),
				pos+1,
				filename.c_str(),
				NULL
			);

		} else {

			// No path, store on root.
			rc = iso_tree_add_new_node(
				image,
				iso_image_get_root(image),
				source.path,
				filename.c_str(),
				NULL
			);

		}

		if(rc < 0) {
			cerr << "iso9660\tError '" << iso_error_to_msg(rc) << "' adding node" << endl;
			throw runtime_error(iso_error_to_msg(rc));
		}

	}

	void iso9660::Worker::save(const char *filename) {

		int fd = open(filename,O_CREAT|O_TRUNC|O_APPEND|O_WRONLY,0666);
		if(fd < 0) {
			throw system_error(errno,system_category(),filename);
		}

		try {

			save(fd);

		} catch(...) {

			::close(fd);
			remove(filename);
			throw;

		}

	}

	void iso9660::Worker::set_system_area(const char *path) {

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

			if(read(fd,data,sizeof(data)) < 1) {
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

	void iso9660::Worker::set_volume_id(const char *volume_id) {

		if(volume_id && *volume_id) {
			iso_image_set_volume_id(image, volume_id);
		} else {
			iso_image_set_volume_id(image, Config::Value<string>("iso9660","volume-id",PACKAGE_NAME).c_str());
		}

	}

	void iso9660::Worker::set_publisher_id(const char *publisher_id) {

		if(publisher_id && *publisher_id) {
			iso_image_set_publisher_id(image, publisher_id);
		} else {
			iso_image_set_publisher_id(image, Config::Value<string>("iso9660","publisher-id",PACKAGE_NAME " " PACKAGE_RELEASE).c_str());
		}

	}

	void iso9660::Worker::set_data_preparer_id(const char *data_preparer_id) {

		if(data_preparer_id && *data_preparer_id) {

			iso_image_set_data_preparer_id(image, data_preparer_id);

		} else {

			char username[32];
			if(getlogin_r(username, 32) == 0) {
				iso_image_set_data_preparer_id(image, username);
			}

		}

	}

	void iso9660::Worker::set_application_id(const char *application_id) {

		if(application_id && *application_id) {

			iso_image_set_application_id(image,application_id);

		} else {

			iso_image_set_application_id(image,Config::Value<string>("iso9660","application-id",PACKAGE_NAME).c_str());

		}

	}

	void iso9660::Worker::set_system_id(const char *system_id) {

		if(system_id && *system_id) {

			iso_image_set_system_id(image,system_id);

		} else {

			iso_image_set_system_id(image,Config::Value<string>("iso9660","system-id","LINUX").c_str());;

		}

	}

	void iso9660::Worker::set_iso_level(int level) {
		iso_write_opts_set_iso_level(opts, level);
	}

	void iso9660::Worker::set_rockridge(int rockridge) {
		iso_write_opts_set_rockridge(opts, rockridge);
	}

	void iso9660::Worker::set_joliet(int joliet) {
		iso_write_opts_set_joliet(opts, joliet);
	}

	void iso9660::Worker::set_allow_deep_paths(int deep_paths) {
		iso_write_opts_set_allow_deep_paths(opts, deep_paths);
	}

	void iso9660::Worker::set_el_torito_boot_image(const char *isopath, const char *catalog, const char *id) {

		ElToritoBootImage *bootimg = NULL;
		int rc = iso_image_set_boot_image(image,isopath,ELTORITO_NO_EMUL,catalog,&bootimg);
		if(rc < 0) {
			cerr << "iso9660\tError '" << iso_error_to_msg(rc) << "' setting el-torito boot image" << endl;
			throw runtime_error(iso_error_to_msg(rc));
		}
		el_torito_set_load_size(bootimg, 4);
		el_torito_patch_isolinux_image(bootimg);
		iso_write_opts_set_part_like_isohybrid(opts, 1);

		{
			uint8_t id_string[28];
			memset(id_string,' ',sizeof(id_string));

			if(id && *id) {
				strncpy((char *) id_string,id,strlen(id));
			} else {
				Config::Value<string> defstring("iso9660","el-torito-id",PACKAGE_NAME);
				strncpy((char *) id_string,defstring,strlen(defstring));
			}

			el_torito_set_id_string(bootimg,id_string);
			cout << "iso9660\tEl-torito ID string set to '" << string((const char *) id_string,28) << "'" << endl;
		}

		// bit0= Patch the boot info table of the boot image. This does the same as mkisofs option -boot-info-table.
		el_torito_set_isolinux_options(bootimg,1,0);

	}

	void iso9660::Worker::add_boot_image(const char *isopath, uint8_t id) {
		ElToritoBootImage *bootimg = NULL;
		int rc = iso_image_add_boot_image(image,isopath,ELTORITO_NO_EMUL,0,&bootimg);
		if(rc < 0) {
			cerr << "iso9660\tError '" << iso_error_to_msg(rc) << "' adding boot image" << endl;
			throw runtime_error(iso_error_to_msg(rc));
		}

		el_torito_set_boot_platform_id(bootimg, id);
	}

	void iso9660::Worker::set_efi_boot_image(const char *boot_image, bool like_iso_hybrid) {

		if(like_iso_hybrid) {

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

	void iso9660::Worker::save(int fd) {

		int rc;

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

		Dialog::Progress &progress = Dialog::Progress::getInstance();
		progress.set_title(_("Writing ISO image"));

		try {

			#define BUFLEN 2048
			unsigned char buffer[BUFLEN];

			double current = 0;
			double total = burn_src->get_size(burn_src);

			while(burn_src->read_xt(burn_src, buffer, BUFLEN) == BUFLEN) {

				if(write(fd,buffer,BUFLEN) != BUFLEN) {
					throw system_error(errno, system_category(),_("I/O error writing image"));
				}

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

		progress.set_title(_("Finalizing"));
		::fsync(fd);

	}

 }
