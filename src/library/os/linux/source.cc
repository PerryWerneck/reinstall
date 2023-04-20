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
 #include <udjat/defs.h>
 #include <reinstall/defs.h>
 #include <reinstall/source.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <udjat/tools/intl.h>
 #include <libgen.h>
 #include <mntent.h>
 #include <udjat/tools/quark.h>
 #include <unistd.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	const char * Source::rpath() const {

		if(!path) {
			throw runtime_error(_("Required attribute 'path' is missing"));
		}

		string mountpoint;
		string dir;

		{
			size_t szBuffer = strlen(path)+2;
			char buffer[szBuffer];
			memset(buffer,0,szBuffer);
			strncpy(buffer,path,szBuffer-1);
			dir = dirname(buffer);
		}

		FILE *aFile = setmntent("/proc/mounts", "r");
		if (aFile == NULL) {
				throw system_error(errno, system_category(),"/proc/mounts");
		}

		struct mntent *ent;
		while (NULL != (ent = getmntent(aFile))) {

			size_t szEnt = strlen(ent->mnt_dir);
			if(szEnt < mountpoint.size() || szEnt > dir.size()) {
				continue;
			}

			if(!strncmp(dir.c_str(),ent->mnt_dir,szEnt)) {
				mountpoint = ent->mnt_dir;
			}

		}
		endmntent(aFile);

		if(mountpoint.empty()) {
			throw runtime_error(_("Cant find valid mountpoint"));
		}

		Logger::String{"Found mountpoint '",mountpoint,"' for '",this->path,"'"}.trace(name());

		if(mountpoint.size() == 1 && mountpoint[0] == '/') {
			return this->path;
		}

		return (this->path + mountpoint.size());

	}

 }

