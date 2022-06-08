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
 #include <udjat/tools/object.h>
 #include <cstring>

 namespace Reinstall {

	class UDJAT_API Object : public Udjat::NamedObject {
	protected:
		struct Link {
			/// @brief URL para o link.
			const char *url = "";

			/// @brief Label para o ícone.
			const char *label = "";

			/// @brief Ícone para o link
			const char *icon_name = "";

			/// @brief Tooltip
			const char *tooltip = "";

			constexpr Link() { }
			Link(const char *tagname, const pugi::xml_node &node);
		};

		/// @brief Texto longo.
		struct Text {

			/// @brief Texto
			const char *body = "";

			/// @brief Tooltip
			const char *tooltip = "";

			constexpr Text() {}
			Text(const char *tagname, const pugi::xml_node &node);

		};

		/// @brief Mensagens a apresentar para o usuário (popup)
		struct Message : public Text {

			/// @brief Título da janela
			const char * title = "";

			/// @brief Emite notificação se a janela não tiver foco?
			bool notify = false;

			Message(const char *tag, const pugi::xml_node &node);

		};

	private:

		/// @brief Label (for menu)
		const char *label = "";

		/// @brief Title of the container.
		const char *title = "";

		/// @brief Subtitle
		Text subtitle;

		/// @brief Link para o botão de help.
		Link help;

	public:
		Object(const pugi::xml_node &node);

	};

 }
