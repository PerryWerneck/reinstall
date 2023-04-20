/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <private/widgets.h>
 #include <udjat/tools/logger.h>

 using namespace Gtk;
 using namespace std;
 using namespace Udjat;

 namespace Widget {

	 Group::Group(const pugi::xml_node &node) : label{node,"title"}, body{node,"sub-title"}, icon{node,Gtk::ICON_SIZE_DND,"",true} {

		set_hexpand(true);
		set_halign(ALIGN_FILL);

		set_vexpand(true);
		set_valign(ALIGN_START);

		label.set_hexpand(true);
		body.set_hexpand(true);

		get_style_context()->add_class("group-box");
		label.get_style_context()->add_class("group-title");

		/*
		if(icon) {
			icon.get_style_context()->add_class("action-icon");
			icon.set_valign(ALIGN_CENTER);
			icon.set_halign(ALIGN_CENTER);
			attach(icon,0,0,1,2);
		}
		*/

		attach(label,1,0,1,1);
		if(body) {
			body.get_style_context()->add_class("group-subtitle");
			attach(body,1,1,2,1);
		}

		set(node);

		actions.get_style_context()->add_class("group-actions");
		attach(actions,1,2,2,1);
		show_all();

	}

	void Group::push_back(std::shared_ptr<Reinstall::Action> action) {

		::Widget::Action *widget = dynamic_cast<::Widget::Action *>(action->get_button().get());
		if(!widget) {
			throw runtime_error(string{"Invalid action push back on group '"} + get_label() + "'");
			return;
		}

		Reinstall::Abstract::Group::push_back(action);

		widget->set_sensitive(action->enabled());

		Glib::signal_idle().connect([this,widget,action](){

			widget->set_active(action->is_selected());

			widget->signal_toggled().connect([widget,action]() {
				if(widget->get_active()) {
					action->set_selected();
				}
			});

			// actions.pack_start(*widget,true,true,0);
			actions.add(*widget);
			return 0;
		});

	}

	std::string Group::get_label() const {
		return label.get_text().c_str();
	}

 }
