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

			/// @brief Repository URL got from XML definition.
			const char * url = "";

			Path(const pugi::xml_node &node);

		} path;

		/// @brief SLP based repository detection.
		class SlpClient {
		private:

			/// @brief The service type string, including the authority string (if any) for the request.
			const char *service_type = "";

			/// @brief A comma separated list of scope names.
			const char *scope = "";

			/// @brief A query formulated of attribute pattern matching expressions in the form of an LDAPv3 search filter.
			const char *filter = "";

			/// @brief URL for kernel parameter when SLP server was detected.
			const char *kparm = "";

			/// @brief User message while detecting.
			const char *message = "";

			/// @brief Allow local address?
			bool allow_local = false;

			/// @brief Resolved url
			std::string url;

		public:
			SlpClient() = default;

			SlpClient(const pugi::xml_node &node);

			inline operator bool() const noexcept {
				return (service_type && *service_type);
			}

			/// @brief Resolve SLP service, return URL
			/// @return SLP response (empty if not found).
			const char * get_url();

		} slp;

	public:

		/// @brief Repository layout.
		const enum Layout : uint8_t {
			ApacheLayout,		///< @brief Repository is a standard apache directory.
			MirrorCacheLayout,	///< @brief It's a MirrorCache repository;
		} layout;

		typedef struct {
			bool operator() (const std::shared_ptr<Repository> a, const std::shared_ptr<Repository> b) const {
				return strcasecmp(a->name(),b->name()) == 0;
			}
		} Equal;

		typedef struct {
			size_t operator() (const std::shared_ptr<Repository> a) const {
				return a->hash();
			}
		} Hash;

		Repository(const pugi::xml_node &node);
		~Repository();

		/// @brief Get repository URL.
		/// @param expand If true resolve the real URL using SLP.
		/// @param Repository URL.
		const std::string get_url(bool expand = false);

	};

 }
