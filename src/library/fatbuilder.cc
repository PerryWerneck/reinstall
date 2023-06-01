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

 // References:
 //
 //		http://elm-chan.org/fsw/ff/00index_e.html
 //


 #include <config.h>
 #include <reinstall/actions/fatbuilder.h>
 #include <reinstall/builder.h>
 #include <udjat/tools/file.h>
 #include <reinstall/dialogs/progress.h>
 #include <ff.h>
 #include <diskio.h>

 #ifndef _GNU_SOURCE
		#define _GNU_SOURCE             /* See feature_test_macros(7) */
 #endif // _GNU_SOURCE

 #include <fcntl.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	FatBuilder::FatBuilder(const pugi::xml_node &node, const char *icon_name)
		: Reinstall::Action(node,icon_name), imglen{getImageSize(node)} {

		if(!imglen) {
			throw runtime_error("Required attribute 'size' is missing or invalid");
		}

	}

	FatBuilder::~FatBuilder() {
	}

	std::shared_ptr<Reinstall::Builder> FatBuilder::BuilderFactory() {

		class Builder : public Reinstall::Builder, private File::Temporary {
		private:
			FATFS fs;

		public:
			Builder(const FatBuilder &action) {
				if(fallocate(fd,0,0,action.imglen)) {
					throw system_error(errno,system_category(),"Cant allocate FAT image");
				}
			}

			void pre(const Action &) override {

				if(disk_ioctl(0, CTRL_FORMAT, &this->fd) != RES_OK) {
					throw runtime_error("Cant bind fatfs to disk image");
				}

				// Format
				{
					static const MKFS_PARM parm = {FM_FAT32, 0, 0, 0, 0};

					BYTE work[FF_MAX_SS];
					memset(work,0,sizeof(work));
					auto rc = f_mkfs("0:", &parm, work, sizeof work);

					if(rc != FR_OK) {
						throw runtime_error(Logger::String{"Unexpected error '",rc,"' on f_mkfs"});
					}

				}

				// Mount
				{
					auto rc = f_mount(&fs, "0:", 1);
					if(rc != FR_OK) {
						throw runtime_error(Logger::String{"Unexpected error '",rc,"' on f_mount"});
					}
				}


			}

			/// @brief Step 2, insert source, download it if necessary.
			/// @return true if source was downloaded.
			bool apply(Source &source) override {

				Dialog::Progress &dialog = Dialog::Progress::getInstance();
				dialog.set_url(source.path);

				string filename{"0:"};

				if(*source.path != '/') {
					filename += '/';
				}

				filename += source.path;

				debug(filename);

				FIL fil;
				memset(&fil,0,sizeof(fil));

				// Create file (and directory), open it ...
				auto rc = f_open(&fil, filename.c_str(), FA_CREATE_NEW | FA_WRITE);
				if(rc == FR_NO_PATH) {

					// Create directory.
					const char *from = filename.c_str();
					const char *to = strchr(from+3,'/');
					while(to) {

						auto res = f_mkdir(string{from,(size_t) (to-from)}.c_str());
						if(!(res == FR_OK || res == FR_EXIST)) {
							throw runtime_error(Logger::String{"Unexpected error '",res,"' on f_mkdir(",string{from,(size_t) (to-from)},")"});
						}

						to = strchr(to+1,'/');
					}

					// try again...
					rc = f_open(&fil, filename.c_str(), FA_CREATE_NEW | FA_WRITE);
				}

				if(rc != FR_OK) {
					throw runtime_error(Logger::String{"Unexpected error '",rc,"' on f_open"});
				}

				debug("Saving '",filename,"'");

				try {

					// ... write file contents ...

					source.save([&fil](const void *buf, size_t length){

						UINT wrote = 0;
						auto rc = f_write(&fil,buf,length,&wrote);
						if(rc != FR_OK) {
							throw runtime_error(Logger::String{"Unexpected error '",rc,"' on f_write"});
						}

						if(wrote != length) {
							throw runtime_error("Unable to write on fat disk");
						}

					});

				} catch(...) {

					f_close(&fil);
					throw;

				}

				// ... and close it
				f_close(&fil);

				return false;

			}

			/// @brief Step 3, build (after downloads).
			void build(Action &action) override {
				throw runtime_error("Incomplete");
			}

			/// @brief Step 4, finalize.
			void post(const Action &action) override {
				auto rc = f_mount(NULL, "", 0);
				if(rc != FR_OK) {
					throw runtime_error(Logger::String{"Unexpected error '",rc,"' on f_umount"});
				}
			}

		};

		return make_shared<Builder>(*this);
	}

	std::shared_ptr<Reinstall::Writer> FatBuilder::WriterFactory() {
		throw runtime_error("Incomplete");
	}


 }

