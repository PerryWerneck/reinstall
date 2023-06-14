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

 // References:
 //
 //		http://elm-chan.org/fsw/ff/00index_e.html
 //

 #include <config.h>
 #include <libreinstall/builder.h>
 #include <libreinstall/dialogs/progress.h>
 #include <udjat/tools/file/temporary.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>

 #ifndef _GNU_SOURCE
	#define _GNU_SOURCE             // See feature_test_macros(7)
 #endif // _GNU_SOURCE

 #include <fcntl.h>
 #include <stdexcept>
 #include <ff.h>
 #include <diskio.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 #include <libreinstall/builders/fat.h>

 using namespace Reinstall;
 using namespace Udjat;
 using namespace std;

 namespace Fat {

	std::shared_ptr<Reinstall::Builder> BuilderFactory(unsigned long long length) {

		class FatBuilder : public Reinstall::Builder, private File::Temporary {
		private:
			FATFS fs;
			bool mounted = false;

		public:
			FatBuilder(unsigned long long length) : Builder{"fatfs"} {

				info("Building disk image with {}",String{}.set_byte(length));

				if(fallocate(this->fd,0,0,length)) {
					throw system_error(errno,system_category(),"Cant allocate FAT image");
				}

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

			}

			virtual ~FatBuilder() {
				if(mounted) {
					post();
				}
			}

			void pre() override {
				auto rc = f_mount(&fs, "0:", 1);
				if(rc != FR_OK) {
					throw runtime_error(Logger::String{"Unexpected error '",rc,"' on f_mount"});
				}
				mounted = true;
			}

			void post() override {
				auto rc = f_mount(NULL, "", 0);
				if(rc != FR_OK) {
					error("Unexpected error '{}' on f_umount",rc);
				}
				fsync(fd);
				mounted = false;
			}

			void push_back(std::shared_ptr<Reinstall::Source::File> file) override {

				if(!mounted) {
					throw runtime_error("Builder is unprepared");
				}

				FIL fil;
				memset(&fil,0,sizeof(fil));

				// Create file (and directory), open it ...
				auto rc = f_open(&fil, file->c_str(), FA_CREATE_NEW | FA_WRITE);
				if(rc == FR_NO_PATH) {

					// Create directory.
					const char *from = file->c_str();
					const char *to = strchr(from+3,'/');
					while(to) {

						auto res = f_mkdir(string{from,(size_t) (to-from)}.c_str());
						if(!(res == FR_OK || res == FR_EXIST)) {
							throw runtime_error(Logger::String{"Unexpected error '",res,"' on f_mkdir(",string{from,(size_t) (to-from)},")"});
						}

						to = strchr(to+1,'/');
					}

					// try again...
					rc = f_open(&fil, file->c_str(), FA_CREATE_NEW | FA_WRITE);

				}

				if(rc != FR_OK) {
					throw runtime_error(Logger::String{"Unexpected error '",rc,"' on f_open(",file->c_str(),")"});
				}

				try {

					debug("Writing ",file->c_str());

					Dialog::Progress &progress = Dialog::Progress::getInstance();

					file->save([&fil,&progress](unsigned long long current, unsigned long long total, const void *buf, size_t length){

						progress.set_progress(current,total);

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

				f_close(&fil);
			}

			void write(std::shared_ptr<Writer> writer) override {
				throw runtime_error("Incomplete");
			}


		};

		return make_shared<FatBuilder>(length);

	}

 }

