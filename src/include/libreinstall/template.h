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

		const char marker = '$';

		/// @brief Escape control chars from values?
		bool escape_values = false;

		/// @brief List of escaped chars
		const char *escape_chars = "&";

		/// @brief The escape char.
		const char *escape_char = "\\";

	public:
		constexpr Template(const char *name, const char *f, const char *u) : Udjat::NamedObject{name}, filter{f}, url{u} {
		}

		Template(const Udjat::XML::Node &node);

		/// @brief Apply object to template
		/// @param object The object to apply
		/// @return The text of the template with applied object.
		Udjat::String apply(const Udjat::Abstract::Object &object) const;

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

	};

 }

