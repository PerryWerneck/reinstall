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

 #include <config.h>
 #include <udjat/tools/object.h>
 #include <reinstall/object.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/logger.h>
 #include <pugixml.hpp>
 #include <iostream>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	pugi::xml_node Abstract::Object::find(const pugi::xml_node &node, const char *attrname) {

		for(pugi::xml_node child = node.child("attribute"); child; child = child.next_sibling("attribute")) {
			const char *name = child.attribute("name").as_string("");
			if(!strcasecmp(name,attrname)) {
				return child;
			}
		}

		Logger::String{"Cant find attribute '",attrname,"'"}.trace("xmlnode");

		return pugi::xml_node();
	}

	void Abstract::Object::set(const pugi::xml_node &node) {

		Udjat::NamedObject::set(node);

		pugi::xml_node child;

		// Get label
		child = find(node,"label");
		if(child) {
			String label{child};
			debug("label=",label.c_str());
			set_label(label.as_quark());
		}

		// Get sub-title
		child = find(node,"sub-title");
		if(child) {
			String body{child};
			debug("body=",body.c_str());
			set_body(body.as_quark());
		}

		// Get dialogs
		for(pugi::xml_node parent = node; parent; parent = parent.parent()) {

			for(auto child = parent.child("dialog"); child; child = child.next_sibling("dialog")) {

				switch(String{child,"name"}.select("confirmation","success","failed",nullptr)) {
				case 0: // confirmation.
					if(!dialog.confirmation) {
						dialog.confirmation.set(child);
					}
					break;
				case 1: // success
					if(!dialog.success) {
						dialog.success.set(child);
					}
					break;
				case 2: // failed
					if(!dialog.failed) {
						dialog.failed.set(child);
					}
					break;
				default:
					warning() << "Unexpected dialog name '" << String{child,"name"} << "'" << endl;
				}

			}

		}

	}

	void Abstract::Popup::set(const pugi::xml_node &node) {

		const char *group = node.attribute("settings-from").as_string("popup-defaults");

		message = Udjat::Object::getAttribute(node,group,"message","");
		url.link = Udjat::Object::getAttribute(node,group,"url","");
		url.label = Udjat::Object::getAttribute(node,group,"url-label",_("More info"));

		{
			Udjat::String text{node.child_value()};
			text.expand(node,group);
			text.strip();
			if(!text.empty()) {
				secondary = text.as_quark();
			}
		}

	}

	/*
	Object::Label::Label(const pugi::xml_node &nd, const char *attrname) : Gtk::Label{"", Gtk::ALIGN_START, Gtk::ALIGN_START} {

		auto node = find(nd,attrname);
		if(!node) {
			debug("Cant find attribute '",attrname,"' for node ",nd.attribute("name").as_string());
			return;
		}

		set_text(Quark{node,"value"}.c_str());

		const char *tooltip = node.attribute("tooltip").as_string();
		if(tooltip && *tooltip) {
			set_tooltip_text(Quark{tooltip}.c_str());
		}

	}

	Object::Link::Link(const pugi::xml_node &nd, const char *attrname) {

		auto node = find(nd,attrname);
		if(!node) {
			valid = false;
			debug("Cant find attribute '",attrname,"' for node ",nd.attribute("name").as_string());
			return;
		}

		valid = true;

		set_relief(Gtk::RELIEF_NONE);
		set_focus_on_click(false);

		set_image_from_icon_name(node.attribute("icon-name").as_string("help-contents"));

		const char *tooltip = node.attribute("tooltip").as_string();
		if(tooltip && *tooltip) {
			set_tooltip_text(Quark{tooltip}.c_str());
		}

	}


	Object::Object(const pugi::xml_node &node) :
		Udjat::NamedObject{node},
		title{node,"title"},
		subtitle(node,"sub-title"),
		help(node,"help-url") {

		subtitle.set_line_wrap(true);
		// subtitle.wrap_mode(Pango::WRAP_WORD);


	}

	bool Object::getProperty(const char *key, std::string &value) const noexcept {

		if(!strcasecmp(key,"title")) {

			value = title.get_text();
			return true;

		} else if(!strcasecmp(key,"sub-title")) {

			value = subtitle.get_text();
			return true;

		}

		return Udjat::NamedObject::getProperty(key,value);
	}

	void Object::set_dialog(Gtk::Window &window) const {
		// https://developer-old.gnome.org/gtkmm/stable/classGtk_1_1Window.html
		window.set_title(title.get_text());
		if(icon && *icon) {
			window.set_icon_name(icon);
		}
	}

	*/

 }

