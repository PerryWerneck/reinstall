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

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/quark.h>
 #include <pugixml.hpp>
 #include <reinstall/object.h>
 #include <reinstall/worker.h>
 #include <list>
 #include <set>
 #include <memory>
 #include <cstring>

 namespace Reinstall {

	class UDJAT_API Action : public Reinstall::Object {
	protected:

		/// @brief File/Folder to copy from repository to image.
		class UDJAT_API Source {

			const char *url;			///< @brief The file URL.
			const char *path;			///< @brief The path inside the image.
			const char *message = "";	///< @brief User message while downloading file.

			Source(const char *u, const char *p) : url(Udjat::Quark(u).c_str()), path(Udjat::Quark(p).c_str()) {
			}

			inline bool operator< (const Source &b) const noexcept {
				return strcasecmp(url,b.url) < 0;
			}

			bool operator> (const Source &b) const noexcept {
				return strcasecmp(url,b.url) > 0;
			}

			bool operator== (const Source &b) const noexcept {
				return strcasecmp(url,b.url) == 0;
			}

			/// @brief Download file.
			//virtual void apply();

		};

		std::set<Source> sources;
		std::list<std::shared_ptr<Worker>> workers;

		Action(const pugi::xml_node &node);

	public:

		virtual void pre();
		virtual void apply();
		virtual void post();

	};

 }
