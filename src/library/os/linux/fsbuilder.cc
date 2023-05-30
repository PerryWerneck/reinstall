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
 #include <reinstall/builder.h>
 #include <reinstall/actions/fsbuilder.h>
 #include <reinstall/diskimage.h>
 #include <reinstall/dialogs/progress.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/xml.h>
 #include <cstdio>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 #ifdef HAVE_FDISK
	#include <libfdisk.h>
 #endif // HAVE_FDISK

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	FSBuilder::FSBuilder(const pugi::xml_node &node, const char *icon_name)
		: Reinstall::Action(node,icon_name), imglen{getImageSize(node)},
			fsname{Quark{node,"filesystem","fat32"}.c_str()},
			part{(PartitionType) XML::StringFactory(node,"partition-type","value","dos").select("none","dos",nullptr)} {

		if(!imglen) {
			throw runtime_error("Required attribute 'size' is missing");
		}

		if(part >= InvalidPartitionType) {
			throw runtime_error("Unexpected value on attribute 'partition-type'");
		}

	}

	FSBuilder::~FSBuilder() {
	}

#ifdef HAVE_FDISK
	static int ask_callback(struct fdisk_context *cxt, struct fdisk_ask *ask, void *data) {

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
#endif // HAVE_FDISK

	std::shared_ptr<Reinstall::Builder> FSBuilder::BuilderFactory() {

		class Builder : public Reinstall::Builder {
		private:
			Disk::Image *disk;
			std::string filename;
			const char *parttype = "0c";
			FSBuilder::PartitionType part = FSBuilder::DosPartition; // FIX-ME

		public:
			Builder(const std::string &fname, const char *fsname, unsigned long long length)
				: disk{new Disk::Image(fname.c_str(),fsname,length)}, filename{fname} {
			}

			virtual ~Builder() {
				if(disk) {
					delete disk;
					disk = nullptr;
				}
#ifndef DEBUG
				remove(filename.c_str());
#endif // DEBUG
			}

			void pre(const Action &) override {
			}

			void post(const Action &) override {
			}

			void build(Action &) override {
			}

			bool apply(Source &source) override {
				disk->insert(source);
				return true;
			}

			std::shared_ptr<Writer> burn(std::shared_ptr<Writer> writer) override {

				int rc = 0;
				char buffer[512];

				Dialog::Progress &progress = Dialog::Progress::getInstance();
				progress.set_sub_title(_("Preparing to write"));
				double imgsize, current = 0;

				// Close disk image
				delete disk;
				disk = nullptr;

				// Burk disk image.
				debug("Fat image='",filename,"'");

				int fdImage = ::open(filename.c_str(),O_RDONLY);
				if(fdImage < 0) {
					throw system_error(errno,system_category(),"Cant open FAT32 image");
				}

				struct stat imgStat;

				if(fstat(fdImage,&imgStat)) {
					int err = errno;
					::close(fdImage);
					throw system_error(err,system_category(),"Cant get FAT32 stat");
				}

				imgsize = imgStat.st_size;

				try {

#ifdef HAVE_FDISK
					if(part != NoPartition) {

						// Create partition on the first sector.
						imgsize += 512;
						std::string device{File::Temporary::create(512)};

						struct fdisk_context *cxt = fdisk_new_context();
						if(!cxt) {
							throw runtime_error("Unexpected error on fdisk_new_context");
						}
						fdisk_set_ask(cxt, ask_callback, NULL);

						struct fdisk_partition *pa = fdisk_new_partition();
						if(!pa) {
							fdisk_unref_context(cxt);
							throw runtime_error("Unexpected error on fdisk_new_partition");
						}

						if(fdisk_assign_device(cxt, device.c_str(), 0)) {
							fdisk_unref_context(cxt);
							fdisk_unref_partition(pa);
							throw runtime_error("Unexpected error on fdisk_new_partition");
						}

						try {

							unsigned int sectorsize = fdisk_get_sector_size(cxt);
							fdisk_disable_dialogs(cxt, 1);

							fdisk_partition_start_follow_default(pa, 1);
							fdisk_partition_partno_follow_default(pa, 0);
							fdisk_partition_end_follow_default(pa, 0);

							fdisk_partition_set_size(pa, imgStat.st_size / sectorsize);
							fdisk_partition_set_partno(pa, 0);

							{
								struct fdisk_parttype *type = fdisk_label_parse_parttype(fdisk_get_label(cxt, NULL), parttype);
								throw runtime_error("Cant parse partition type");
								fdisk_partition_set_type(pa, type);
								fdisk_unref_parttype(type);
							}

							errno = - fdisk_add_partition(cxt, pa, NULL);
							if(errno) {
								throw system_error(errno,system_category(),"Cant write partition");
							}

							if(fdisk_write_disklabel(cxt)) {
								throw runtime_error("Unexpected error on fdisk_write_disklabel");
							}

						} catch(...) {

							fdisk_deassign_device(cxt, 1);
							fdisk_unref_context(cxt);
							fdisk_unref_partition(pa);
							throw;

						}

						fdisk_deassign_device(cxt, 1);
						fdisk_unref_context(cxt);
						fdisk_unref_partition(pa);

						// Write partition.
						progress.set_sub_title(_("Writing partition table"));

					}

#endif // HAVE_FDISK


					throw runtime_error("Incomplete");

				} catch(...) {

					::close(fdImage);
					throw;

				}

			}

		};


		return make_shared<Builder>(File::Temporary::create(),fsname,imglen);

	}

	std::shared_ptr<Reinstall::Writer> FSBuilder::WriterFactory() {
		debug("Returning USB writer");
		return Reinstall::Writer::USBWriterFactory(*this);
	}

	bool FSBuilder::interact() {
		return true;
	}

 }

