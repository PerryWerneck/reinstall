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

	void iso9660::Worker::push_back(Action::Source &source) {

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
				throw logic_error("Unexpanded path in source list");
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
		progress.set("Writing ISO image");

		try {

			#define BUFLEN 2048
			unsigned char buffer[BUFLEN];

			double current = 0;
			double total = burn_src->get_size(burn_src);

			while(burn_src->read_xt(burn_src, buffer, BUFLEN) == BUFLEN) {

				if(write(fd,buffer,BUFLEN) != BUFLEN) {
					throw system_error(errno, system_category(),"I/O error writing image");
				}

				current += BUFLEN;
				if(total) {
					progress.update(current,total);
				}

			}


		} catch(...) {

			burn_src->free_data(burn_src);
			throw;

		}

		burn_src->free_data(burn_src);
		free(burn_src);

		progress.set("Finalizing");
		::fsync(fd);

	}

 }
