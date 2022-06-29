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

 #include <udjat/defs.h>
 #include <udjat/tools/value.h>
 #include <unordered_map>
 #include <string>

 namespace Reinstall {

	class UDJAT_API Parameters : public Udjat::Value {
	private:

		class Value : public std::string, public Udjat::Value {
		private:
			Type type = String;

		public:
			Value(const pugi::xml_node &node);
			virtual ~Value();

			bool isNull() const override;
			Value & reset(const Type type = Undefined) override;
			Value & set(const Value &value) override;
			Value & set(const char *value, const Type type = String) override;
			const Value & get(std::string &value) const override;

		};

		std::unordered_map<std::string,Value> values;

	public:
		Parameters(const pugi::xml_node &node, const char *tagname);

		virtual ~Parameters();

		/// @brief Has any value?
		bool isNull() const override;

		/// @brief Navigate from all values.
		void for_each(const std::function<void(const char *name, const Value &value)> &call) const override;

		/// @brief Clear contents, set value type.
		Value & reset(const Type type = Undefined) override;

		/// @brief Convert value to 'object' and insert child.
		Value & set(const Value &value) override;

	};

 }


