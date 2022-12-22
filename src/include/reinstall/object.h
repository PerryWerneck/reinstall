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
 #include <gtkmm.h>
 #include <glibmm/i18n.h>
 #include <string>

 namespace Reinstall {

	class UDJAT_API Object : public Udjat::NamedObject {
	public:
		class UDJAT_API Label : public Gtk::Label {
		public:
			Label(const pugi::xml_node &node, const char *attrname);

			inline bool empty() const {
				return get_text().empty();
			}

			inline operator bool() const {
				return !get_text().empty();
			}

		};

		/// @brief Title of the container.
		Label title;

		/// @brief Subtitle
		Label subtitle;

		/*
		class UDJAT_API Link : public Gtk::LinkButton {
		public:
			Link(const pugi::xml_node &node);

			inline const char * c_str() const {
				return get_uri().c_str();
			}

			inline bool empty() const {
				return get_uri().empty();
			}

			inline operator bool() const {
				return !get_uri().empty();
			}
		};

		/// @brief Help button.
		Link help;

		*/

	protected:

		/*
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
		*/

		/// @brief Texto longo.
		struct Text {

			/// @brief Texto
			const char *body = "";

			/// @brief Tooltip
			const char *tooltip = "";

			constexpr Text() {}
			Text(const char *tagname, const pugi::xml_node &node);

		};

		/*
		/// @brief Mensagens a apresentar para o usuário (popup)
		struct Message : public Text {

			/// @brief Título da janela
			const char * title = "";

			/// @brief Emite notificação se a janela não tiver foco?
			//bool notify = false;

			Message(const char *tag, const pugi::xml_node &node);

		};
		*/

		bool getProperty(const char *key, std::string &value) const noexcept override;

	public:

		Object(const pugi::xml_node &node);

	};

 }

 namespace std {

	inline string to_string(const Reinstall::Object::Label &label) {
		return string{label.get_text()};
	}

	inline ostream& operator<< (ostream& os, const Reinstall::Object::Label &label) {
			return os << to_string(label);
	}

 }

