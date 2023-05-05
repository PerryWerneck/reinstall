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
 #include <reinstall/action.h>
 #include <list>
 #include <memory>
 #include <functional>

 namespace Reinstall {

	namespace Abstract {

		class UDJAT_API Group : public Reinstall::Abstract::Object {
		private:
			std::list<std::shared_ptr<Action>> actions;

		public:

			unsigned short id;

			Group();
			virtual ~Group();

			virtual void push_back(std::shared_ptr<Action> action);

			/// @brief Navigate from groups.
			bool for_each(const std::function<bool (std::shared_ptr<Action> action)> &call) const;

			static std::shared_ptr<Group> find(const pugi::xml_node &node);
			static std::shared_ptr<Group> find(const unsigned short id);
			static std::shared_ptr<Group> factory(const pugi::xml_node &node);

		};

	}

 }
