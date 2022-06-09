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
 #include <pugixml.hpp>
 #include <reinstall/object.h>
 #include <reinstall/worker.h>
 #include <list>
 #include <set>
 #include <memory>
 #include <cstring>
 #include <functional>

 namespace Reinstall {

	class UDJAT_API Action : public Reinstall::Object {
	public:
		/// @brief File/Folder to copy from repository to image.
		class UDJAT_API Source {
		public:
			const char *url;		///< @brief The file URL.
			const char *path;		///< @brief The path inside the image.
			const char *message;	///< @brief User message while downloading file.

			Source(const pugi::xml_node &node);

			/// @brief Check if it's required to download the source.
			inline bool local() const noexcept {
				return (*path != 0);
			}

			inline bool operator< (const Source &b) const noexcept {
				return strcasecmp(path,b.path) < 0;
			}

			bool operator> (const Source &b) const noexcept {
				return strcasecmp(path,b.path) > 0;
			}

			bool operator== (const Source &b) const noexcept {
				return strcasecmp(path,b.path) == 0;
			}

			/// @brief Download file.
			//virtual void apply();

		};

	protected:
		std::set<std::shared_ptr<Source>> sources;
		std::list<std::shared_ptr<Worker>> workers;

		/// @brief Scan xml for 'tagname', call lambda in every occurrence.
		/// @param tagname the <tag> to search for.
		/// @param node the start node.
		/// @param call The method to call on every tag (returning 'true' stop the search).
		/// @return true if the 'call' has returned true.
		bool scan(const pugi::xml_node &node, const char *tagname, const std::function<bool(const pugi::xml_node &node)> &call);

		/// @brief Get list of sources from 'tagname'.
		void scanForSources(const pugi::xml_node &node, const char *tagname);

		Action(const pugi::xml_node &node);

	public:
		unsigned short id;

		virtual ~Action();

		virtual void pre();
		virtual void apply();
		virtual void post();

	};

 }
