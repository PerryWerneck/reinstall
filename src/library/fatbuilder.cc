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

	}

	FatBuilder::~FatBuilder() {
	}

	std::shared_ptr<Reinstall::Builder> FatBuilder::BuilderFactory() {

		class Builder : public Reinstall::Builder, private File::Temporary {
		private:

		public:
			Builder(const FatBuilder &action) {

				debug("---------------------- FATFS builder imglen=",action.imglen);

				if(action.imglen && fallocate(fd,0,0,action.imglen)) {
					throw system_error(errno,system_category(),"Cant allocate FAT image");
				}

				if(disk_ioctl(0, CTRL_FORMAT, &this->fd) != RES_OK) {
					throw runtime_error("Cant bind fatfs to disk image");
				}

				/*
				// Create partition
				{
					BYTE work[FF_MAX_SS];
					memset(work,0,sizeof(work));
					LBA_t plist[] = {50, 50, 0};

					f_fdisk(0, plist, work);
				}
				*/

				// Format
				{
					BYTE work[FF_MAX_SS];
					memset(work,0,sizeof(work));
					auto rc = f_mkfs("0:", 0, work, sizeof work);

					if(rc != FR_OK) {
						throw runtime_error(Logger::String{"Unexpected error '",rc,"' on fatfs.mkfs"});
					}

				}

#ifdef DEBUG
				link("/tmp/fatfs.bin");
#endif // DEBUG


				throw runtime_error("Incomplete");

			}

			void pre(const Action &action) override {
			}

			/// @brief Step 2, insert source, download it if necessary.
			/// @return true if source was downloaded.
			bool apply(Source &source) override {
			}

			/// @brief Step 3, build (after downloads).
			void build(Action &action) override {
			}

			/// @brief Step 4, finalize.
			void post(const Action &action) override {
			}

		};

		return make_shared<Builder>(*this);
	}

	std::shared_ptr<Reinstall::Writer> FatBuilder::WriterFactory() {
		throw runtime_error("Incomplete");
	}


 }

