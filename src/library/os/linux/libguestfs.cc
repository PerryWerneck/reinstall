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
 #include <reinstall/diskimage.h>
 #include <stdexcept>
 #include <iostream>

 using namespace std;

 #ifdef HAVE_GUESTFS

 #include <guestfs.h>

 namespace Reinstall {

	/*
	struct Disk::Image::Handler {

		guestfs_h *g;

		Handler() : g(guestfs_create()) {
			if(!g) {
				throw runtime_error("Error in guestfs_create");
			}
		}

		~Handler() {
			if(guestfs_umount_all(g) == -1) {
				cerr << "guestfs\tError on guestfs umount" << endl;
			}

			if(guestfs_shutdown(g) == -1) {
				cerr << "guestfs\tError on guestfs shutdown" << endl;
			}

			guestfs_close(g);

			cout << "guestfs\tGuestfs handle closed" << endl;
		}

	};

	Disk::Image::Image(const char *filename) {

		handler = new Handler();

		if(guestfs_add_drive(handler->g, filename) == -1) {
			throw runtime_error(string{"Error in guestfs_add_drive ("} + filename + ")");
		}

		if(guestfs_launch(handler->g) == -1) {
			throw runtime_error("Error in guestfs_launch");
		};

		cout << "guestfs\tFile '" << filename << "' mounted" << endl;
	}

	Disk::Image::~Image() {
		delete handler;
	}

	void Disk::Image::forEach(const std::function<void (const char *filename)> &call) {

	}
	*/

 }

 #endif // HAVE_GUESTFS
