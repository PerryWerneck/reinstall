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
 #include <libreinstall/source.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/intl.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 #include <fcntl.h>
 #include <stdexcept>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	Source::Source(const Udjat::XML::Node &node) :
		name{  Quark{ node,"name",node.name() }.c_str() }, path{ node }, slpclient{ node }, imgpath{ XML::QuarkFactory(node,"path").c_str() } {
	}

	const char * Source::local() const noexcept {

		if(path.local && *path.local && access(path.local,R_OK) == 0) {
			return path.local;
		}

		return nullptr;
	}

	const char * Source::remote() const {

		if(slpclient) {

			// SLP is enabled, search it.
			const char *url = slpclient.resolve();
			if(url && *url) {
				return url;
			}

		}
#ifdef DEBUG
		else {
			Logger::String("No SLP service defined, using url").warning(name);
		}
#endif // DEBUG

		if(path.remote && *path.remote) {
			return path.remote;
		}

		throw runtime_error(Logger::Message{_("Can't determine source URL for '{}'"),name});

	}

 }
