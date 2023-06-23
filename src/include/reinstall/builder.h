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

 /*
 #pragma once

 #include <pugixml.hpp>
 #include <udjat/defs.h>
 #include <reinstall/action.h>
 #include <reinstall/writer.h>

 namespace Reinstall {

	class UDJAT_API Builder {
	protected:
		const char *name = "builder";

	public:

		Builder();
		virtual ~Builder();

		/// @brief Step 1, initialize image (before downloads).
		virtual void pre(const Action &action) = 0;

		/// @brief Step 2, insert source, download it if necessary.
		/// @return true if source was downloaded.
		virtual bool apply(Source &source);

		/// @brief Step 3, build (after downloads).
		virtual void build(Action &action) = 0;

		/// @brief Step 4, finalize.
		virtual void post(const Action &action) = 0;

		/// @brief Step 5 (last step, work thread).
		/// @param Write prepared image @see Action::WriterFactory
		virtual std::shared_ptr<Writer> burn(std::shared_ptr<Writer> writer);

		/// @brief Get image length.
		/// @return Image length in bytes.
		/// @retval 0 Image length is not available.
		virtual size_t size();

	};

 }
 */
