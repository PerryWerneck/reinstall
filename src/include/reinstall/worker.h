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

 #include <pugixml.hpp>
 #include <udjat/defs.h>
 #include <reinstall/action.h>

 namespace Reinstall {

	class UDJAT_API Worker {
	protected:

		/// @brief Insert source on the target image.
		virtual void push_back(Source &source);

	public:

		Worker();

		virtual void pre(Action &action);
		virtual void apply(Action &action);
		virtual void post(Action &action);

	};

 }
