/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2022 Perry Werneck <perry.werneck@gmail.com>
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
 #include <reinstall/defs.h>
 #include <reinstall/writer.h>
 #include <reinstall/diskimage.h>
 #include <system_error>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>

#ifndef _WIN32
 #include <unistd.h>
#endif // _WIN32

#ifdef HAVE_FDISK
 #include <libfdisk.h>
#endif // HAVE_FDISK

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	Writer::Writer(const Reinstall::Action UDJAT_UNUSED(&action)) {
	}

	Writer::~Writer() {
	}

	void Writer::setUsbDeviceName(const char *name) {
		usbdevname = name;
	}

	void Writer::setUsbDeviceLength(unsigned long long length) {
		usbdevlength = length;
	}

	void Writer::open() {
	}

	void Writer::write(const void UDJAT_UNUSED(*buf), size_t UDJAT_UNUSED(count)) {
		throw system_error(ENOTSUP,system_category(),"Module is trying to write on dummy writer");
	}

	void Writer::finalize() {
	}

	void Writer::close() {
	}

	std::shared_ptr<Disk::Image> Writer::DiskImageFactory(const char *devname, const char *fsname) {
		return std::make_shared<Disk::Image>(devname,fsname);
	}

#ifdef HAVE_FDISK
	static int ask_callback(struct fdisk_context *, struct fdisk_ask *ask, void *) {

		switch(fdisk_ask_get_type(ask)) {
		case FDISK_ASKTYPE_INFO:
			Logger::String{fdisk_ask_print_get_mesg(ask)}.info("fdisk");
			break;

		case FDISK_ASKTYPE_WARNX:
			Logger::String{fdisk_ask_print_get_mesg(ask)}.error("fdisk");
			break;

		case FDISK_ASKTYPE_WARN:
			Logger::String{fdisk_ask_print_get_mesg(ask),": ",strerror(fdisk_ask_print_get_errno(ask))}.error("fdisk");
			break;

		default:
			break;
		}

		return 0;
	}

	void Writer::make_partition(int fd, uint64_t length, const char *parttype) {

		int rc = 0;
		ssize_t dataoffset = 1048576;	// 2048 sectors

		// Am I writing to block device?
		{
			struct stat st;
			if(fstat(fd, &st)) {
				throw system_error(errno,system_category(),"Cant get device stat");
			}

			if((st.st_mode & S_IFMT) == S_IFREG) {

				// It's a regular file, allocate it.

				Logger::String{"Writing to image file"}.info("writer");

				uint64_t sectors = (length/512)+1;
				uint64_t required_length = dataoffset+(sectors * 512);

				debug("Required sectors=",sectors,"Required length=",required_length);
				if(fallocate(fd,0,0,required_length)) {
					throw system_error(errno,system_category(),"Cant allocate disk space");
				}

				// Just in case.
				lseek(fd,0,SEEK_SET);

			}

		}

		// Create partition on the first sector.
		struct fdisk_context *cxt = fdisk_new_context();
		if(!cxt) {
			throw runtime_error("Unexpected error on fdisk_new_context");
		}
		fdisk_set_ask(cxt, ask_callback, NULL);

		// https://cdn.kernel.org/pub/linux/utils/util-linux/v2.28/libfdisk-docs/libfdisk-Context.html#fdisk-enable-wipe
		fdisk_enable_wipe(cxt,1);

		struct fdisk_partition *pa = fdisk_new_partition();
		if(!pa) {
			fdisk_unref_context(cxt);
			throw runtime_error("Unexpected error on fdisk_new_partition");
		}
		fdisk_partition_size_explicit(pa,1);

		/*

		TODO: Find why fdisk_assign_device_by_fd doesnt work.

		errno = - fdisk_assign_device_by_fd(cxt, fd, "diskimage", 0);
		if(errno) {
			fdisk_unref_context(cxt);
			fdisk_unref_partition(pa);
			throw system_error(errno,system_category(),"fdisk_assign_device_by_fd has failed");
		}
		*/

		{
			// For some reason assing by fd doesnt work, then, I'm using the filename.
			char fn[4097];

			ssize_t sz = readlink(String{"/proc/self/fd/",fd}.c_str(),fn,4096);
			if(sz < 0) {
				throw system_error(errno,system_category(),"Cant get device path");
			}

			Logger::String{"Writing partition table to '",fn,"'"}.trace("fdisk");

			fn[sz] = 0;
			rc = fdisk_assign_device(cxt, fn, 0);
			if(rc) {
				fdisk_unref_context(cxt);
				fdisk_unref_partition(pa);
				throw system_error(-rc,system_category(),"fdisk_assign_device has failed");
			}

		}

		if(fdisk_create_disklabel(cxt, "dos")) {
			fdisk_unref_context(cxt);
			fdisk_unref_partition(pa);
			throw runtime_error("Unexpected error on fdisk_create_disklabel");
		}

		try {

			// https://cdn.kernel.org/pub/linux/utils/util-linux/v2.28/libfdisk-docs/libfdisk-Partition.html

			uint64_t sectorsize = fdisk_get_sector_size(cxt);
			uint64_t sectors = length / sectorsize;
			fdisk_disable_dialogs(cxt, 1);

			debug("sectors=",sectors," sectorsize=",sectorsize);

			//fdisk_partition_start_follow_default(pa, 1);
			//fdisk_partition_end_follow_default(pa, 1);
			//fdisk_partition_partno_follow_default(pa, 1);

			// http://sft.if.usp.br/linux/utils/util-linux/v2.31/libfdisk-docs/libfdisk-Partition.html#fdisk-partition-set-start
			fdisk_partition_start_follow_default(pa, 0);
			if(fdisk_partition_set_start(pa, dataoffset/sectorsize) < 0) {
				throw runtime_error("Unexpected error on fdisk_partition_set_start");
			}

			if(fdisk_partition_set_size(pa, sectors) < 0) {
				throw runtime_error("Unexpected error on fdisk_partition_set_size");
			}
			fdisk_partition_end_follow_default(pa, 0);

			fdisk_partition_partno_follow_default(pa, 0);
			if(fdisk_partition_set_partno(pa, 0) < 0) {
				throw runtime_error("Unexpected error on fdisk_partition_set_partno");
			}

			{
				struct fdisk_parttype *type = fdisk_label_parse_parttype(fdisk_get_label(cxt, NULL), parttype);
				if(!type) {
					throw runtime_error(String{"Cant parse partition type '",parttype,"'"});
				}
				fdisk_partition_set_type(pa, type);
				fdisk_unref_parttype(type);
			}

			Logger::String{
				"Partition ",
				fdisk_partition_get_partno(pa),
				" from sector ",
				fdisk_partition_get_start(pa),
				" to sector ",
				fdisk_partition_get_end(pa),
				" with ",
				fdisk_partition_get_size(pa),
				" sectors of ",
				sectorsize,
				" bytes"
			}.trace("fdisk");

			dataoffset = fdisk_partition_get_start(pa) * sectorsize;

			errno = - fdisk_add_partition(cxt, pa, NULL);
			if(errno) {
				debug("errno=",errno);
				throw system_error(errno,system_category(),"Cant add partition");
			}

			// https://cdn.kernel.org/pub/linux/utils/util-linux/v2.35/libfdisk-docs/libfdisk-Label.html#fdisk-write-disklabel
			rc = fdisk_write_disklabel(cxt);
			if(rc) {
				throw system_error(-rc,system_category(),"fdisk_write_disklabel has failed");
			}

			/*
			if(Logger::enabled(Logger::Trace)) {

				struct fdisk_parition *pa;
				char *data;

				fdisk_get_partition(cxt, 0, &pa);
				fdisk_partition_to_string(pa, FDISK_FIELD_UUID, &data);

				if(data && *data) {
					Logger::String{data}.trace("fdisk");

				}
				free(data);
				fdisk_unref_partition(pa);

			}
			*/

		} catch(...) {

			fdisk_deassign_device(cxt, 1);
			fdisk_unref_context(cxt);
			fdisk_unref_partition(pa);
			throw;

		}

		fdisk_deassign_device(cxt, 1);
		fdisk_unref_context(cxt);
		fdisk_unref_partition(pa);

		if(lseek(fd,dataoffset,SEEK_SET) != dataoffset) {
			throw system_error(ENOENT,system_category(),"Cant set file pointer");
		}

	}

#else

	void Writer::make_partition(uint64_t length, const char *parttype) {
		throw runtime_error("Unsupported call to 'make_partition'");
	}

#endif // HAVE_FDISK

 }
