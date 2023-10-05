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
 #include <udjat/defs.h>
 #include <gtkmm.h>
 #include <glibmm/i18n.h>
 #include <udjat/tools/xml.h>
 #include <udjat/ui/dialog.h>
 #include <udjat/ui/menu.h>
 #include <udjat/ui/gtk/label.h>
 #include <udjat/ui/gtk/icon.h>
 #include <udjat/factory.h>
 #include <memory>
 #include <vector>

 class UDJAT_PRIVATE MainWindow : public Gtk::Window, private Udjat::Menu::Controller, private Udjat::Dialog::Controller {
 private:

	class Logo : public Gtk::Image {
	public:
		Logo();
		void set(const char *name);
	} logo;

 	struct {
		Gtk::Label title{ _( "Select option" ), Gtk::ALIGN_START };
		Gtk::Box hbox;
		Gtk::Box vbox{Gtk::ORIENTATION_VERTICAL};
		Gtk::Box view{Gtk::ORIENTATION_VERTICAL};
		Gtk::ScrolledWindow swindow;
		Gtk::ButtonBox bbox;
 	} layout;

 	struct {
		Gtk::Button apply{_("_Apply"), true};
		Gtk::Button cancel{_("_Cancel"), true};
 	} buttons;

	void set_icon_name(const char *icon_name);
	void set_icon(const char *name);

	class Group : public Gtk::Box {
	private:
		Udjat::Label label;
		Udjat::Label subtitle;
		Gtk::LinkButton linkbutton;
		Gtk::Box title{Gtk::ORIENTATION_HORIZONTAL};

	public:
		Group(const Udjat::XML::Node &node);

		void push_back(Gtk::Widget &child);

	};

	/// @brief The window groups.
	std::vector<std::shared_ptr<Group>> groups;

	/// @brief The active group.
	std::shared_ptr<Group> group;

	class Item : public ::Gtk::RadioButton {
	private:
		Udjat::Menu::Item *menu;
		Udjat::Label title;
		Udjat::Label subtitle;
		Udjat::Gtk::Icon icon;
		::Gtk::Grid grid;

	public:
		::Gtk::LinkButton help;

		Item(Udjat::Menu::Item *menu, const Udjat::XML::Node &node);

		inline std::string get_text() const {
			return title.get_text();
		}

		inline void activate() {
			menu->activate();
		}

	};

	std::vector<std::shared_ptr<Item>> items;

	std::shared_ptr<Item> selected;

 public:

	class PropertyFactory;
	friend class PropertyFactory;

	MainWindow();
	virtual ~MainWindow();

	void push_back(Udjat::Menu::Item *menu, const Udjat::XML::Node &node) override;

	int run(const Udjat::Dialog &dialog, const std::vector<Udjat::Dialog::Button> &buttons) override;
	int run(const Udjat::Dialog &dialog, const std::function<int(Udjat::Dialog::Progress &progress)> &task) override;
	int run(const Udjat::Dialog &dialog, const std::function<int(Udjat::Dialog::Popup &popup)> &task, const std::vector<Udjat::Dialog::Button> &) override;

	std::shared_ptr<Group> find(const pugi::xml_node &node, const char *attrname);

 };

