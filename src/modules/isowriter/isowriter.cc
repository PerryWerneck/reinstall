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
 #include <iostream>
 #include <reinstall/dialogs/progress.h>
 #include <reinstall/builder.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>

 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 using namespace std;
 using namespace Udjat;
 using namespace Reinstall;

 IsoWriter::IsoWriter(const pugi::xml_node &node) : Reinstall::Action(node,"drive-removable-media") {

 	if(!sources.empty()) {
		Logger::String{"Discarding ",sources.size()," sources"}.warning(name());
 	}

	sources.clear(); // Remove other sources.
	sources.insert(make_shared<Source>(node));

 }

 std::shared_ptr<Reinstall::Builder> IsoWriter::BuilderFactory() {

	class Builder : public Reinstall::Builder {
	private:
		int fd = -1;

	public:
		Builder() = default;

		~Builder() {
			if(fd > 0) {
				debug("Closing device");
				::close(fd);
				debug("Device closed");
				fd = -1;
			}
		}

		size_t size() override {

			debug("fd=",fd);

			if(fd < 0) {
				cerr << "IsoWriter\tController asked for size of unavailable image, returning '0'" << endl;
			}

			struct stat statbuf;
			if(fstat(fd,&statbuf) != 0) {
				cerr << "isowriter\tCan't get image file size: " << strerror(errno) << endl;
				return 0;
			}

			return statbuf.st_size;
		}

		void pre(const Reinstall::Action &) override {
		}

		bool apply(Reinstall::Source &source) override {

			if(fd > 0) {
				throw runtime_error(_("More sources than module expects"));
			}

			if(!Reinstall::Builder::apply(source)) {
				return false;
			}

			source.save();

			fd = ::open(source.filename(),O_RDONLY);
			if(fd < 0) {
				throw system_error(errno, system_category(), _("Cant access downloaded image"));
			}

			Logger::String{"ISO File is '",source.filename(),"'"}.write(Logger::Trace,"isowriter");

			return true;
		}

		void build(Reinstall::Action &) override {
		}

		void post(const Reinstall::Action &) override {
		}

		std::shared_ptr<Reinstall::Writer> burn(std::shared_ptr<Reinstall::Writer> writer) override {

			Reinstall::Dialog::Progress &progress = Reinstall::Dialog::Progress::getInstance();
			progress.set_sub_title(_("Writing ISO image"));

			size_t current = 0;
			size_t total = size();
			if(!size()) {
				throw runtime_error(_("Unable to get image size"));
			}

			#define BUFLEN 2048
			unsigned char buffer[BUFLEN];

			while(current < total) {

				size_t length = (total - current);
				if(length > BUFLEN) {
					length = BUFLEN;
				}

				ssize_t bytes = ::read(fd, buffer, length);
				if(bytes < 0) {
					throw system_error(errno,system_category(),_("Cant read from image file"));
				}

				if(bytes == 0) {
					throw runtime_error(_("Unexpected EOF reading image file"));
				}

				writer->write(buffer,length);

				current += length;
				progress.set_progress(current,total);

			}

			progress.set_sub_title(_("Finalizing"));
			writer->finalize();

			return writer;
		}


	};

	return(make_shared<Builder>());

 }

