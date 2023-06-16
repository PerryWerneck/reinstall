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

 /**
  * @brief Brief description of this source.
  */

 #include <config.h>
 #include <udjat/tools/xml.h>
 #include <libreinstall/repository.h>

 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	Repository::Repository(const Udjat::XML::Node &node)
		:	name{XML::QuarkFactory(node,"name").c_str()}, slpclient{node},
			remote{XML::QuarkFactory(node,"remote").c_str()},
			local{XML::QuarkFactory(node,"local").c_str()},
			kparm{node} {

		if(!(name && *name)) {
			throw runtime_error("Required attribute 'name' is missing or invalid");
		}

		if(!(remote && *remote)) {
			remote = XML::QuarkFactory(node,"url").c_str();
		}

		if(!(remote && *remote)) {
			throw runtime_error("Required attribute 'remote' is missing or invalid");
		}

	}

	Udjat::URL Repository::url() const {

		Repository *repo = const_cast<Repository *>(this);

		if(slpclient) {
			const char *url = slpclient.resolve();
			if(url && *url) {
				Logger::String{"Using slp URL ",remote}.trace(name);
				if(repo) {
					repo->kparm.val = kparm.slpval;
				}
				return Udjat::URL{url};
			}
		}

		Logger::String{"Using default URL ",remote}.trace(name);
		if(repo) {
			repo->kparm.val = remote;
		}

		return Udjat::URL{remote};
	}

	void Repository::set_kernel_parameter(const Udjat::XML::Node &node) {

		if(kparm) {
			Logger::String{"Kernel parameter was already set to '",kparm.name()}.warning(name);
		}

		kparm.set_name(XML::QuarkFactory(node,"name").c_str());

	}

 }
