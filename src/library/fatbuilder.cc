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
 #include <reinstall/actions/fatbuilder.h>
 #include <reinstall/builder.h>
 #include <udjat/tools/file.h>

 #ifndef _GNU_SOURCE
		#define _GNU_SOURCE             /* See feature_test_macros(7) */
 #endif // _GNU_SOURCE

 #include <fcntl.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	class UDJAT_PRIVATE FatBuilder::Disk : public Udjat::File::Temporary {
	public:
		Disk(unsigned long long imglen) {

			if(imglen && fallocate(fd,0,0,imglen)) {
				throw system_error(errno,system_category(),"Cant allocate FAT image");
			}


		}
	};

	FatBuilder::FatBuilder(const pugi::xml_node &node, const char *icon_name)
		: Reinstall::Action(node,icon_name) {

	}

	FatBuilder::~FatBuilder() {
	}

	std::shared_ptr<Reinstall::Builder> FatBuilder::BuilderFactory() {

		class Builder : public Reinstall::Builder {
		private:
			std::shared_ptr<FatBuilder::Disk> disk;

		public:
			Builder(const FatBuilder &action) : disk{make_shared<FatBuilder::Disk>(action.imglen)} {
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

