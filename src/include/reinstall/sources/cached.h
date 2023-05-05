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

 #pragma once

 #include <udjat/defs.h>
 #include <reinstall/source.h>
 #include <pugixml.hpp>
 #include <udjat/tools/application.h>
 #include <stdexcept>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	/// @brief Source for file in application cache.
	class UDJAT_API CachedFileSource : public Source {
	protected:
		bool cache = true;

	public:
		CachedFileSource(const pugi::xml_node &node, const char *defmessage = nullptr) : Source(node), cache{getAttribute(node,"cache",true)} {
			if(!(url && *url)) {
				throw runtime_error(_("Required attribute 'URL' is missing"));
			}
			if(!(message && *message)) {
				message = defmessage;
			}
		}

		void save() override {
			Reinstall::Source::save(Udjat::Application::CacheDir().build_filename(url).c_str());
		}

	};

 }
