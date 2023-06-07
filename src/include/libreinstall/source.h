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
 #include <udjat/tools/xml.h>

 namespace Reinstall {

	/// @brief Common data source.
	class UDJAT_API Source {
	private:

		const struct Path {

			/// @brief URL for remote repository.
			const char * remote = "";

			/// @brief Path for the local copy of repository.
			const char * local = "";

			constexpr Path(const char *r, const char *l) : remote{r}, local{l} {
			}

			Path(const Udjat::XML::Node &node);

		} path;

		/// @brief SLP based repository detection.
		const struct SlpClient {

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

			constexpr SlpClient() = default;

			SlpClient(const Udjat::XML::Node &node);

			inline operator bool() const noexcept {
				return (service_type && *service_type);
			}

			/// @brief Do SLP query, get first valid URL.
			/// @return The detected URL (empty string if no url was detected).
			const char *resolve() const;

		} slpclient;

	protected:

		/// @brief Path in the target image.
		const char *imgpath = "";

	public:

		/// Build Source using fixed values.
		/// @param remote URL of the root for remote source.
		/// @param local Path for source in the local file system.
		/// @param path Path for source in the target image.
		constexpr Source(const char *remote, const char *local = "", const char *path = "") : path{remote,local}, imgpath{path} {
		}

		Source(const Udjat::XML::Node &node);

	};

 }

