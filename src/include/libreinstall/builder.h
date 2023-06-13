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
 #include <udjat/tools/object.h>
 #include <libreinstall/source.h>
 #include <libreinstall/template.h>
 #include <memory>
 #include <vector>

 namespace Reinstall {

	class Writer;

	/// @brief The abstract image builder.
	class UDJAT_API Builder : public Udjat::NamedObject {
	public:
		constexpr Builder(const char *name) : Udjat::NamedObject{name} {
		}

		/// @brief Start build.
		virtual void pre() = 0;

		/// @brief Finalize build.
		virtual void post() = 0;

		/// @brief Add file to image.
		virtual void push_back(std::shared_ptr<Reinstall::Source::File> file) = 0;

		/// @brief Apply template on image.
		virtual void push_back(const Udjat::Abstract::Object &object, const std::vector<Reinstall::Template> &tmpls);

		/// @brief Write image.
		virtual void write(std::shared_ptr<Writer> writer) = 0;

	};

 }

