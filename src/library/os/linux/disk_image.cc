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

 #ifndef _GNU_SOURCE
	#define _GNU_SOURCE             /* See feature_test_macros(7) */
 #endif // !_GNU_SOURCE

 #include <config.h>
 #include "private.h"
 #include <reinstall/diskimage.h>
 #include <reinstall/dialogs.h>
 #include <udjat/tools/file.h>
 #include <stdexcept>
 #include <iostream>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <linux/loop.h>
 #include <sys/ioctl.h>
 #include <sys/mount.h>
 #include <dirent.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/string.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	// @brief Abstract disk image handler.
	class Disk::Image::Handler {
	public:

		/// @brief Loop device.
		Device::Loop device;

		/// @brief Mountpoint.
		std::string mountpoint;

		Handler(const char *imgpath) : device{imgpath}, mountpoint{File::Temporary::mkdir()} {
		}

		~Handler() {
			rmdir(mountpoint.c_str());
		}

	};


	static const struct Worker {
		const char *name;
		const char *fsname;
		const std::function<void(const char *dev, unsigned long long szimage)> format;
	} workers[] = {
		{
			"fat32",
			"vfat",
			[](const char *dev, unsigned long long szimage) {

				Logger::String{"Creating fat32 image '",dev,"' with ",String{}.set_byte(szimage).c_str()}.trace("disk");

				int fd = ::open(dev,O_CREAT|O_TRUNC|O_WRONLY,0644);
				if(fd < 0) {
					throw system_error(errno,system_category(),"Cant create FAT32 image");
				}

				if(fallocate(fd,0,0,szimage)) {
					int err = errno;
					::close(fd);
					throw system_error(err,system_category(),"Cant allocate FAT32 image");
				}

				::close(fd);

				// Format.
				SubProcess{
					Logger::Message{
						Config::Value<string>{
							"mkfs","fat32","/sbin/mkfs.vfat -F32 {}"
						}.c_str(),
						dev
					}.c_str()
				}.run();

			}
		},
		{
			"udf",
			"udf",
			[](const char *dev, unsigned long long szimage) {

				unsigned long long blocksize = 2048LL;
				unsigned long long blocks = (szimage/blocksize)+1;

				Logger::String{"Creating fat32 image with ",blocks," blocks of ",blocksize," bytes"}.trace("disk");

				SubProcess{
					Logger::Message{
						Config::Value<string>{
							"mkfs","udf","/usr/sbin/mkfs.udf {} --blocksize={} {}"
						}.c_str(),
						dev,
						std::to_string(blocksize).c_str(),
						std::to_string(blocks).c_str()
					}.c_str()
				}.run();

			}
		}
	};

	const Worker & WorkerFactory(const char *filesystemtype) {

		for(const Worker &worker : workers) {

			if(strcasecmp(filesystemtype,worker.name) == 0) {
				return worker;
			}

		}

		throw runtime_error("Invalid filesystemtype");
	}

	Disk::Image::Image(const char *filename, const char *filesystemtype, unsigned long long szimage) {

		const Worker &worker = WorkerFactory(filesystemtype);

		if(szimage) {
			worker.format(filename,szimage);
		}

		handler = new Handler(filename);

		if(mount(handler->device.c_str(), handler->mountpoint.c_str(), Config::Value<string>{"fsname",filesystemtype,worker.fsname}.c_str(), MS_NOATIME|MS_NODIRATIME, "") == -1) {
			delete handler;
			throw system_error(errno, system_category(),Logger::String{"Cant mount ",filesystemtype," image using ",worker.fsname," filesystem"});
		}

		cout << "disk\tFile '" << filename << "' mounted on " << handler->mountpoint << endl;

	}

	Disk::Image::~Image() {

		debug("Destroying disk image");

		for(size_t i = 0; i < 20000; i++) {

			if(::umount(handler->mountpoint.c_str())) {
				cout << "disk\tDevice '" << handler->device.c_str() << "' umounted" << endl;
				break;
			}

			clog << "disk\tError '" << strerror(errno) << "' (rc=" << errno << ") umounting " << handler->device.c_str() << " ("  << i << "/" << 200 << ")" << endl;
			usleep(100);

		}

		delete handler;
	}

	void Disk::Image::forEach(const std::function<void (const char *mountpoint, const char *path)> &call) {
		forEach(handler->mountpoint.c_str(),nullptr,call);
	}

	void Disk::Image::copy(const char *from, const char *to) {

		string filename{handler->mountpoint};

		if(*to != '/') {
			filename += '/';
		}

		filename += to;

		Dialog::Progress &dialog = Dialog::Progress::getInstance();

		dialog.set_url(to);

		{
			const char *str = filename.c_str();
			const char *ptr = strrchr(str,'/');
			if(!ptr) {
				throw runtime_error("Unexpected filename");
			}

			std::string dirname{str,(size_t) (ptr-str)};
			File::Path::mkdir(dirname.c_str());
		}

		File::copy(from,filename.c_str(),[&dialog](double current, double total){
			dialog.set_progress(current,total);
			return true;
		});

	}

	void Disk::Image::forEach(const char *mountpoint, const char *path, const std::function<void (const char *mountpoint, const char *path)> &call) {

		DIR *dir;

		if(path && *path) {
			dir = opendir( (string{mountpoint} + "/" + path).c_str() );
		} else {
			path = "";
			dir = opendir(mountpoint);
		}

		if(!dir) {
			throw system_error(errno, system_category(),path);
		}

		try {

			for(struct dirent *entry = readdir(dir); entry; entry = readdir(dir)) {

				if(entry->d_name[0] == '.')
					continue;

				if(entry->d_type == DT_DIR) {

					forEach(mountpoint,(string{path}+"/"+entry->d_name).c_str(),call);

				} else {

					call(mountpoint,(string{path}+"/"+entry->d_name).c_str());

				}


			}

		} catch(...) {

			closedir(dir);
			throw;

		}

		closedir(dir);

	}

	void Disk::Image::insert(Reinstall::Source &source) {

		string filename{handler->mountpoint};

		if(*source.path != '/') {
			filename += '/';
		}

		filename += source.path;

		Dialog::Progress &dialog = Dialog::Progress::getInstance();
		dialog.set_url(source.path);

		debug(source.path," -> ",filename);

		{
			const char *str = filename.c_str();
			const char *ptr = strrchr(str,'/');
			if(!ptr) {
				throw runtime_error("Unexpected filename");
			}

			std::string dirname{str,(size_t) (ptr-str)};
			File::Path::mkdir(dirname.c_str());
		}

		source.save(filename.c_str());

	}

 }


