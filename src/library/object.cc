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
 #include <iostream>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	pugi::xml_node find(const pugi::xml_node &node, const char *attrname) {

		for(pugi::xml_node child = node.child("attribute"); child; child = child.next_sibling("attribute")) {
			const char *name = child.attribute("name").as_string("");
			if(!strcasecmp(name,attrname)) {
				return child;
			}
		}

		Logger::String{"Cant find attribute '",attrname,"'"}.trace("xmlnode");

		return pugi::xml_node();
	}

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

/*
               GtkWidget *button = gtk_button_new_from_icon_name(link.icon_name.c_str(),GTK_ICON_SIZE_BUTTON);
                gtk_button_set_relief(GTK_BUTTON(button),GTK_RELIEF_NONE);
                gtk_widget_set_focus_on_click(button,FALSE);

                gtk_widget_set_tooltip_text(button,(link.tooltip.empty() ? link.url : link.tooltip).c_str());

                g_signal_connect(button,"clicked",G_CALLBACK(url_clicked),(gpointer) gdk_atom_intern(link.url.c_str(),FALSE));

                gtk_box_pack_start(GTK_BOX(box),GTK_WIDGET(button),FALSE,FALSE,0);

*/

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

		for(pugi::xml_node parent = node; parent; parent = parent.parent()) {

			for(auto child = parent.child("dialog"); child; child = child.next_sibling("dialog")) {

				switch(String{child,"name"}.select("confirmation","success","failed",nullptr)) {
				case 0: // confirmation.
					if(!confirmation) {
						confirmation.setup(child);
					}
					break;
				case 1: // success
					if(!success) {
						success.setup(child);
					}
					break;
				case 2: // failed
					if(!failed) {
						failed.setup(child);
					}
					break;
				default:
					warning() << "Unexpected dialog name '" << String{child,"name"} << "'" << endl;
				}

			}

		}

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

	void Object::Popup::setup(const pugi::xml_node &node) {

		const char *group = node.attribute("settings-from").as_string("popup-defaults");

		message = getAttribute(node,group,"message","");
		url.link = getAttribute(node,group,"url","");
		url.label = getAttribute(node,group,"url-label",_("More info"));

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

