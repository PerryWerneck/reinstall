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
 #include <reinstall/action.h>
 #include <reinstall/userinterface.h>
 #include <pugixml.hpp>

 class UDJAT_PRIVATE MainWindow : public Gtk::Window, private Reinstall::UserInterface {
 private:

 	class Logo : public Gtk::Image {
	public:
		Logo();
 	};

	Logo logo;

 	struct {
		Gtk::Label title{ _( "Select option" ), Gtk::ALIGN_START };
		Gtk::Box hbox, vbox{Gtk::ORIENTATION_VERTICAL}, view{Gtk::ORIENTATION_VERTICAL};
		Gtk::ScrolledWindow swindow;
		Gtk::ButtonBox bbox;
 	} layout;

 	struct {
		Gtk::Button apply{_("_Apply"), true}, cancel{_("_Cancel"), true};
 	} buttons;

	void on_show() override;

	void apply();

 public:

	MainWindow();
	virtual ~MainWindow();

	/// @brief Construct an action button.
	std::shared_ptr<Reinstall::Abstract::Object> ActionFactory(const pugi::xml_node &node, const char *icon_name) override;

	/// @brief Construct a filename (gui thread).
	std::string FilenameFactory(const char *title, const char *label_text, const char *apply, const char *filename, bool save) override;

	/// @brief Construct a group box.
	std::shared_ptr<Reinstall::Abstract::Group> GroupFactory(const pugi::xml_node &node) override;

	/// @brief Construct a wait for task dialog.
	std::shared_ptr<Reinstall::Dialog::TaskRunner> TaskRunnerFactory(const char *message, bool markup) override;

 };

