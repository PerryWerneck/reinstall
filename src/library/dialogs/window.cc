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
 #include <stdexcept>
 #include <udjat/tools/intl.h>
 #include <reinstall/dialogs.h>

 using namespace std;

 namespace Reinstall {

	namespace Dialog {

		Window * Window::current = nullptr;

		Window::Window() : parent(current) {
			current = this;
			if(parent) {
				parent->hide();
			}
		}

		Window::~Window() {
			if(current != this) {
				runtime_error(_("Window order mismatch"));
			}
			current = parent;
			if(current) {
				current->show();
			}
		}

		Window * Window::getInstance() {
			if(!current) {
				throw runtime_error(_("No active window"));
			}
			return current;
		}

		void Window::set(const Object &object) {
		}

		void Window::show() {
		}

		void Window::hide() {
		}

	}

 }
