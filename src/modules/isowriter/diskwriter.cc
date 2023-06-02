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
 #include "private.h"

 #include <reinstall/source.h>
 #include <reinstall/action.h>
 #include <reinstall/writer.h>
 #include <iostream>
 #include <reinstall/dialogs/progress.h>
 #include <reinstall/builder.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <udjat/tools/quark.h>
 #include <reinstall/diskimage.h>

 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 using namespace std;
 using namespace Udjat;
 using namespace Reinstall;

 DiskWriter::DiskWriter(const pugi::xml_node &node) : Reinstall::Action(node,"drive-removable-media"), fsname{Quark{node,"filesystem","fat32"}.c_str()} {
 }

 std::shared_ptr<Reinstall::Builder> DiskWriter::BuilderFactory() {

	class Builder : public Reinstall::Builder {
	public:
		Builder() {
		}

		void pre(const Action &) override {
		}

		void post(const Action &) override {
		}

		void build(Action &) override {
		}

		bool apply(Source &) override {
			return true;
		}

	};

	return make_shared<Builder>();

 }

 void DiskWriter::post(std::shared_ptr<Reinstall::Writer> writer)  {

	Dialog::Progress &progress = Dialog::Progress::getInstance();

	progress.set_sub_title(_("Formatting image"));

	writer->format(fsname);

	progress.set_sub_title(_("Writing image"));

	auto disk = writer->DiskImageFactory(fsname);

	size_t item = 0;
	for(std::shared_ptr<Source> source : sources) {

		progress.set_count(++item,sources.size());
		progress.set_url(source->url);
		disk->insert(*source);

	}

 }

