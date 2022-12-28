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
 #include <udjat/defs.h>
 #include <reinstall/tools.h>
 #include <dbus/dbus.h>
 #include <iostream>

 using namespace std;

 namespace Reinstall {

	UDJAT_API void reboot() noexcept {

		// Ask gnome for reboot.
		//
		// http://askubuntu.com/questions/15428/reboot-without-sudoer-privileges
		//
		// Logout: dbus-send --session --type=method_call --print-reply --dest=org.gnome.SessionManager /org/gnome/SessionManager org.gnome.SessionManager.Logout uint32:1
		//
		// Reboot: dbus-send --session --type=method_call --print-reply --dest=org.gnome.SessionManager /org/gnome/SessionManager org.gnome.SessionManager.Reboot
		//
		// Shutdown: dbus-send --session --type=method_call --print-reply --dest=org.gnome.SessionManager /org/gnome/SessionManager org.gnome.SessionManager.Shutdown
		//
		DBusError		  err;
		DBusConnection	* dbus_connection = dbus_bus_get(DBUS_BUS_SESSION,NULL);

		dbus_error_init(&err);
		dbus_connection	= dbus_bus_get(DBUS_BUS_SESSION,&err);

		if(dbus_connection) {

			DBusMessage	* dbus_message =
					dbus_message_new_method_call(
						"org.gnome.SessionManager",		// Destination
						"/org/gnome/SessionManager",	// Path
						"org.gnome.SessionManager",		// Interface
						"Reboot"						// Method
					);

			dbus_connection_set_exit_on_disconnect(dbus_connection,FALSE);
			dbus_connection_send(dbus_connection,dbus_message,NULL);
			dbus_connection_flush(dbus_connection);

		} else {

			cerr << "Error '";

			if(dbus_error_is_set(&err)) {
				cerr << err.message;
				dbus_error_free(&err);
			} else {
				cerr << "unexpected";
			}

			cerr << "' while asking gnome for reboot" << endl;

#ifndef DEBUG
			system("/sbin/reboot");
#endif // DEBUG
		}

	}

 }

