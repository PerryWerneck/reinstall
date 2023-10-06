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
  * @brief Declare image template object.
  */

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <libreinstall/source.h>
 #include <set>
 #include <udjat/tools/object.h>
 #include <udjat/tools/string.h>

 namespace Reinstall {

	class UDJAT_API Template : public Udjat::NamedObject {
	private:

		/// @brief Template filename, starting with '/' for fullpath.
		const char *filter = nullptr;

		/// @brief URL for template file, file:// for local file.
		const char *url = nullptr;

		/// @brief Template path on destination image.
		const char *filename = "";

		char marker = '$';

	public:
		constexpr Template(const char *name, const char *f, const char *u) : Udjat::NamedObject{name}, filter{f}, url{u} {
		}

		Template(const Udjat::XML::Node &node);

		/// @brief Get template path.
		/// @return The 'path' attribute with the template path in the destination image.
		/// @retval "" The template has no 'path' attribute
		inline const char *path() const noexcept {
			return filename;
		}

		/// @brief Test if string matches template filter.
		/// @param path Path to match.
		/// @return true if template matches the path.
		bool test(const char *path) const noexcept;

		/// @brief Build source from object and image path.
		/// @param object The object for ${} expansion.
		/// @param path The image path for source.
		std::shared_ptr<Reinstall::Source::File> SourceFactory(const Udjat::Abstract::Object &object, const char *path) const;

		/// @brief Load template.
		Udjat::String get(const Udjat::Abstract::Object &object) const;

	};

 }

