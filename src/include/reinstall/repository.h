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
 #include <udjat/tools/object.h>
 #include <memory>
 #include <pugixml.hpp>

 namespace Reinstall {

	/// @brief Installation repository defined by XML.
	class UDJAT_API Repository : public Udjat::NamedObject {
	private:

		struct Path {

			/// @brief Defined URL.
			const char * url;

			Path(const pugi::xml_node &node);

		} path;

		/// @brief SLP based repository detection.
		class SlpClient {
		private:
			const char *service_type;
			const char *scope;
			const char *filter;

		public:
			SlpClient(const pugi::xml_node &node);

			inline operator bool() const noexcept {
				return (service_type && *service_type);
			}

		} slp;

	public:

		typedef struct {
			bool operator() (const std::shared_ptr<Repository> a, const std::shared_ptr<Repository> b) const {
				return strcasecmp(a->name(),b->name()) == 0;
			}
		} Equal;

		typedef struct {
			size_t operator() (const std::shared_ptr<Repository> a) const {
				// return a->hash();
				size_t rc = 5381;
				for (const signed char *p = (const signed char *) a->name(); *p != '\0'; p++)
					rc = (rc << 5) + rc + *p;
				return rc;
			}
		} Hash;

		Repository(const pugi::xml_node &node);
		~Repository();

		/// @brief Get repository URL.
		/// @param expand If true resolve the real URL using SLP.
		const char * url(bool expand = false);

	};

 }
