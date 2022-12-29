/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2022 Perry Werneck <perry.werneck@gmail.com>
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

 #include <config.h>
 #include <gtkmm.h>
 #include <glibmm/i18n.h>
 #include <reinstall/dialogs/popup.h>
 #include <private/dialogs.h>

 Dialog::Popup::Popup(Gtk::Window &parent, const Reinstall::Abstract::Object &object, const Reinstall::Dialog::Popup &settings, Gtk::MessageType type, Gtk::ButtonsType buttons)
	: Gtk::MessageDialog{parent,settings.message,false,type,buttons,true} {

	set_default_size(500, -1);
	set_title(object.get_label());

	url.get_style_context()->add_class("popup-button");
	get_message_area()->add(url);

	show_all();

	set(settings);
 }

 void Dialog::Popup::set(const Reinstall::Dialog::Popup &settings) {

	if(settings.secondary) {
		set_secondary_text(settings.secondary);
	}

	if(settings.url) {
		url.set_uri(settings.url.link);
		url.set_label(settings.url.label);
		url.show();
	} else {
		url.hide();
	}

 }

