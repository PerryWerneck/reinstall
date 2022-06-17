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

 #include <sys/stat.h>
 #include <fcntl.h>

 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 #define LIBISOFS_WITHOUT_LIBBURN
 #include <libisofs/libisofs.h>

 using namespace std;

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
			throw runtime_error("Cant create iso image");
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

		IsoDir * dir = iso_image_get_root(image);

		while(*dirname) {

			string dn;

			const char *next = strchr(dirname,'/');
			if(next) {
				dn = string(dirname,next-dirname);
				dirname = next+1;
			} else {
				dn = string(dirname);
				dirname += dn.size();
			}

			IsoNode * node = NULL;
			int rc = iso_image_dir_get_node(image,dir,dn.c_str(),&node,0);

			if(rc == 0) {

				// Not found, add it.
				iso_tree_add_new_dir(dir, dn.c_str(), (IsoDir **) &node);
				dir = (IsoDir *) node;

			} else {

				// Found, use it.
				dir = (IsoDir *) node;

			}

		}


		return dir;

	}

	void iso9660::Worker::push_back(Action::Source &source) {

		if(!(source.path && *source.path)) {
			// No local path, ignore it.
			return;
		}

		// Download and apply files.
		string filename;

		cout << "iso9660\t" << source.url << " -> " << source.path << endl;

		if(source.local()) {

			// Download and save.
			filename = source.save();

		}

		auto pos = strrchr(source.path,'/');
		if(pos) {

			// Has path, get iso dir.

			iso_tree_add_new_node(
				image,
				getIsoDir(image,string(source.path,pos - source.path).c_str()),
				pos+1,
				filename.c_str(),
				NULL
			);

		} else {

			// No path, store on root.

			iso_tree_add_new_node(
				image,
				iso_image_get_root(image),
				source.path,
				filename.c_str(),
				NULL
			);

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

		try {

			#define BUFLEN 2048
			unsigned char buffer[BUFLEN];

			uint64_t current = 0;
			uint64_t total = burn_src->get_size(burn_src);
			while(burn_src->read_xt(burn_src, buffer, BUFLEN) == BUFLEN) {

				if(write(fd,buffer,BUFLEN) != BUFLEN) {
					throw system_error(errno, system_category(),"I/O error writing image");
				}

				current += BUFLEN;
				//if(total) {
				//	activity.setProgress(current,total,true);
				//}

			}


		} catch(...) {

			burn_src->free_data(burn_src);
			throw;

		}

		burn_src->free_data(burn_src);
		free(burn_src);

/*

		#define BUFLEN 2048
		unsigned char * buffer = new unsigned char[BUFLEN];
		try {

			uint64_t current = 0;
			uint64_t total = burn_src->get_size(burn_src);
			while(burn_src->read_xt(burn_src, buffer, BUFLEN) == BUFLEN) {

				if(write(fd,buffer,BUFLEN) != BUFLEN) {
					throw system_error(errno, system_category(),"Erro ao gravar imagem");
				}

				current += BUFLEN;
				if(total) {
					activity.setProgress(current,total,true);
				}

			}

			activity.setProgress(total,total,true);

		} catch(...) {

			burn_src->free_data(burn_src);
			free(burn_src);

			delete[] buffer;
			throw;
		}
		burn_src->free_data(burn_src);
		free(burn_src);
		delete[] buffer;


*/

		::fsync(fd);

	}

 }
