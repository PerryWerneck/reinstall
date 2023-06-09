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
 #include <libreinstall/source.h>
 #include <memory>
 #include <libreinstall/builder.h>
 #include <udjat/tools/object.h>

 namespace Reinstall {

	/// @brief Standard action.
	class UDJAT_API Action : public Udjat::NamedObject {
	private:
		static Action *selected;	/// @brief Selected action.
		static Action *def;			/// @brief Default action.

	protected:

		/// @brief List of sources defined by XML.
		std::vector<Reinstall::Source> sources;

		/// @brief Get required files.
		virtual void prepare(std::set<std::shared_ptr<Reinstall::Source::File>> &files);

		/// @brief Get Image builder.
		virtual std::shared_ptr<Builder> BuilderFactory() const = 0;

	public:
		Action(const XML::Node &node);
		~Action();

		enum ActivationType {
			Selected,
			Default
		};

		static void activate(const const ActivationType type);

		virtual void activate() const;

		inline void select() noexcept {
			selected = this;
		}

	};

 }
