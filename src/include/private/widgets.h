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

 #include <config.h>
 #include <gtkmm.h>
 #include <glibmm/i18n.h>
 #include <reinstall/group.h>
 #include <reinstall/object.h>
 #include <reinstall/action.h>

 namespace Widget {

 	class Icon : public Gtk::Image {
	public:
		Icon(const pugi::xml_node &node, const Gtk::IconSize iconsize = Gtk::ICON_SIZE_BUTTON, const char *def = "", const bool symbolic = false);
		operator bool() const;

		void set_name(const char *name);

 	};

	class Label : public Gtk::Label {
	public:

		Label(const pugi::xml_node &node, const char *attrname, Gtk::Align halign = Gtk::ALIGN_START, Gtk::Align valign=Gtk::ALIGN_START);

		operator bool() const {
			return get_text()[0] != 0;
		}

	};

 	class Group : public Gtk::Grid, public Reinstall::Abstract::Group {
	private:
		Label label, body;
		Icon icon;
		Gtk::Box actions{Gtk::ORIENTATION_VERTICAL};

	public:
		Group(const pugi::xml_node &node);

		void push_back(std::shared_ptr<Reinstall::Action> action) override;

		std::string get_label() const override;
	};

	class Action : public Gtk::RadioButton, public Reinstall::Abstract::Object {
	private:
		static Gtk::RadioButton::Group group;
		Gtk::Grid grid;
		Gtk::LinkButton help_button;
		Label label, body;
		Icon icon;

	public:
		Action(const pugi::xml_node &node, const char *icon_name);

		std::string get_label() const override;

	};

 }
