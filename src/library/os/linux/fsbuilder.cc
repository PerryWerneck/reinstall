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
 #include <reinstall/writer.h>
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

				Dialog::Progress &progress = Dialog::Progress::getInstance();
				progress.set_sub_title(_("Preparing to write"));

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

				writer->open();

#ifdef HAVE_FDISK
				if(part != NoPartition) {

					// Create partition.
					progress.set_sub_title(_("Writing partition table"));
					writer->make_partition(imgStat.st_size);

				}
#endif // HAVE_FDISK

				// Write image.
				{
					progress.set_sub_title(_("Writing system image"));

					debug("fs-length=",imgStat.st_size," bytes");

					int64_t current = 0;
					char buffer[2048];
					while(current < imgStat.st_size) {

						progress.set_progress(current,imgStat.st_size);
						ssize_t bytes = read(fdImage,buffer,2048);
						if(bytes < 0) {
							throw system_error(errno,system_category(),"Error reading FS image");
						} else if(bytes == 0) {
							throw runtime_error("Unexpected EOF reading FS image");
						}

						writer->write(buffer,bytes);
						current += bytes;

					}

				}

				progress.set_sub_title(_("Finalizing"));

				writer->finalize();
				writer->close();

				progress.set_sub_title(_(""));

				return writer;

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

