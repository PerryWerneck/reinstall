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
 #include <reinstall/group.h>
 #include <list>
 #include <memory>
 #include <udjat/factory.h>

 namespace Reinstall {

	class UDJAT_API Controller : Udjat::Factory {
	private:

		/// @brief Lista de grupos registrados.
		std::list<std::shared_ptr<Group>> groups;

		Controller();

	public:

		static Controller & getInstance();

		/// @brief Factory a new group from XML node.
		/// @param XML definition for the new element.
		/// @return true if the node whas inserted.
		bool push_back(const pugi::xml_node &node) override;

		/// @brief Navigate from groups.
		bool for_each(const std::function<bool (std::shared_ptr<Group> group)> &call) const;

		/// @brief Insert new group.
		inline void push_back(std::shared_ptr<Group> group) {
			groups.push_back(group);
		}

		std::shared_ptr<Group> group();

	};

 }
