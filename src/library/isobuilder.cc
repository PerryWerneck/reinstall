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

 #include "private.h"
 #include <reinstall/isobuilder.h>
 #include <libisofs/libisofs.h>

 namespace Reinstall {

	class UDJAT_PRIVATE IsoBuilderSingleTon {
	private:
		IsoBuilderSingleTon() {
			cout << "IsoBuilder\tStarting iso builder" << endl;
			iso_init();
		}

	public:
		static IsoBuilderSingleTon &getInstance() {
			static IsoBuilderSingleTon instance;
			return instance;
		}

		~IsoBuilderSingleTon() {
			iso_finish();
			cout << "IsoBuilder\tIso builder was terminated" << endl;
		}

	};

	IsoBuilder::IsoBuilder() {

		IsoBuilderSingleTon::getInstance();

		if(!iso_image_new("name", &image)) {
			throw runtime_error("Cant create iso image");
		}

		iso_image_attach_data(image,this,NULL);

		iso_write_opts_new(&opts, 2);
		iso_write_opts_set_relaxed_vol_atts(opts, 1);
		iso_write_opts_set_rrip_version_1_10(opts,1);

	}

	IsoBuilder::~IsoBuilder() {
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

	void IsoBuilder::push_back(Action::Source &source) {

		if(!(source.path && *source.path)) {
			// No local path, ignore it.
			return;
		}

		// Expand folders (ends with '/')
		if( *(source.url + strlen(source.url) - 1) == '/') {

			cerr << "isobuilder\tExpanding " << source.url << endl;

			if( *(source.path + strlen(source.path) - 1) == '/') {
				cerr << "isobuilder\tPath " << source.path << " is not a folder" << endl;
				throw system_error(EINVAL,system_category(),source.path);
			}



		}

		// Download and apply files.
		string filename;

		cout << "isobuilder\t" << source.url << " -> " << source.path << endl;

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


 }
