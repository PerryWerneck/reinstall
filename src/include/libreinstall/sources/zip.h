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
  * @brief Brief Declare zip source class.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <libreinstall/source.h>

 namespace Reinstall {

	/// @brief Common data source.
	class UDJAT_API ZipSource : public Reinstall::Source {
	public:
		constexpr ZipSource(const char *name, const char *remote, const char *local = "", const char *path = "")
			: Reinstall::Source{name,remote,local,path} {
		}

		ZipSource(const Udjat::XML::Node &node)
			: Reinstall::Source{node} {
		}

		/// @brief Open zip file, extract all file names.
		/// @param local URL for local files.
		/// @param remote URL for remote files.
		/// @param files container for the list of file handlers.
		void prepare(const Udjat::URL &local, const Udjat::URL &remote, Source::Files &files) const override;

	};

 }
