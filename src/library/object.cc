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
 #include <reinstall/object.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/string.h>

 using namespace Udjat;

 namespace Reinstall {

	Object::Label::Label(const pugi::xml_node &node, const char *attrname) : Gtk::Label{getAttribute(node,attrname,""), Gtk::ALIGN_START, Gtk::ALIGN_CENTER} {

		const char *tooltip = getAttribute(node,"tooltip","");
		if(tooltip && *tooltip) {
			set_tooltip_text(tooltip);
		}

	}

	/*
	Object::Link::Link(const pugi::xml_node &node) : Gtk::LinkButton{getAttribute(node,"url","")} {

		const char *text;

		text = getAttribute(node,"icon-name","");
		if(text && *text) {
			// set_icon_name(text); FIX-ME
		} else {
			text = getAttribute(node,"label","");
			if(text) {
				set_label(text);
			}
		}

		text = getAttribute(node,"tooltip","");
		if(text && *text) {
			set_tooltip_text(text);
		}

	}
	*/

	Object::Object(const pugi::xml_node &node) :
		Udjat::NamedObject{node},
		title{node,"title"},
		subtitle(node,"sub-title") {

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

	/*
	Object::Link::Link(const char *tagname, const pugi::xml_node &node) {

		const pugi::xml_node & child = node.child(tagname);
		if(!child) {
			return;
		}

		// Obtém grupo do arquivo de configuração para os defaults.
		const char *group = child.attribute("settings-from").as_string("link-defaults");

		url = getAttribute(child,group,"url",url);
		label = getAttribute(child,group,"label",label);
		icon_name = getAttribute(child,group,"icon",icon_name);
		tooltip = getAttribute(child,group,"tooltip",tooltip);

	}

	Object::Text::Text(const char *tag, const pugi::xml_node &node) {

		auto text_node = node.child(tag);
		if(!text_node) {
			// TODO: Upsearch
			return;
		}

		// Obtém grupo do arquivo de configuração para os defaults.
		// const char *group = node.attribute("settings-from").as_string("message-defaults");

		body = Quark(Udjat::String(text_node.child_value()).expand(text_node)).c_str();

	}
	*/

	/*
	Object::Message::Message(const char *tag, const pugi::xml_node &node) : Text(tag,node) {

		auto child = node.child(tag);
		if(!child) {
			return;
		}

		// Obtém grupo do arquivo de configuração para os defaults.
		const char *group = child.attribute("settings-from").as_string("message-defaults");

		title = getAttribute(child,group,"title",title);
		notify = getAttribute(child,group,"notify",false);

	}
	*/

 }

