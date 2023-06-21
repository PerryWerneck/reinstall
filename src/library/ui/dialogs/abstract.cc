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

 /**
  * @brief Implements abstract dialog.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/ui/dialog.h>
 #include <udjat/ui/dialogs/progress.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <vector>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	void Dialog::setup(const XML::Node &node) {

		icon = XML::QuarkFactory(node,"icon-name").c_str();
		title = XML::QuarkFactory(node,"title").c_str();
		message = XML::QuarkFactory(node,"message").c_str();
		secondary = XML::QuarkFactory(node,"secondary").c_str();

		if(!(secondary && *secondary)) {
			Udjat::String text{node.child_value()};
			text.strip();
			if(!text.empty()) {
				text.expand(node,node.attribute("settings-from").as_string("dialog-defaults"));
				auto lines = text.split("\n");
				text.clear();
				for(String &line : lines) {
					line.strip();
					if(!text.empty()) {
						text += "\n";
					}
					text += line;
				}
				secondary = text.as_quark();
			}
		}
	}

	bool Dialog::setup(const char *name, const XML::Node &node) {

		for(XML::Node child = node.child("dialog"); child; child = child.next_sibling("dialog")) {
			if(strcasecmp(node.attribute("name").as_string("*"),name) == 0) {
				setup(child);
				return true;
			}
		}

		return false;
	}

	int Dialog::run(const std::vector<Dialog::Button> &) {
		return -1;
	}

	int Dialog::run(const std::function<int(Progress &progress)> &task) const {
		Dialog::Progress dialog;
		dialog.set(*this);
		return task(dialog);
	}


 }
