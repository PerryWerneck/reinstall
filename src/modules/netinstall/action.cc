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

 #include "private.h"
 #include <memory>
 #include <reinstall/actions/kernel.h>
 #include <reinstall/actions/initrd.h>
 #include <reinstall/isobuilder.h>

 using namespace std;
 using namespace Reinstall;

 namespace NetInstall {

	Action::Action(const pugi::xml_node &node) : Reinstall::Action(node) {

		if(title) {
			title.get_style_context()->add_class("action-title");
		}

		if(subtitle) {
			subtitle.get_style_context()->add_class("action-subtitle");
		}

		// Get URL for installation kernel.
		if(!scan(node,"kernel",[this](const pugi::xml_node &node) {
			push_back(make_shared<Kernel>(node));
			return true;
		})) {
			throw runtime_error("Missing required entry <kernel> with the URL for installation kernel");
		}

		// Get URL for installation init.
		if(!scan(node,"init",[this](const pugi::xml_node &node) {
			push_back(make_shared<InitRD>(node));
			return true;
		})) {
			throw runtime_error("Missing required entry <init> with the URL for the linuxrc program");
		}

#ifdef DEBUG
		info() << "Net Install action constructed" << endl;
#endif // DEBUG

	}

	Action::~Action() {
#ifdef DEBUG
		info() << "Net Install action deleted" << endl;
#endif // DEBUG
	}

	void Action::activate() {

		IsoBuilder worker;

		this->Reinstall::Action::activate(worker);

	}

 }
