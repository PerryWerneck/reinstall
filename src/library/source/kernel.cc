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
 #include <reinstall/defs.h>
 #include <reinstall/source.h>
 #include <reinstall/sources/kernel.h>
 #include <pugixml.hpp>
 #include <udjat/tools/intl.h>
 #include <iostream>
 #include <udjat/tools/logger.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	Kernel::Kernel(const pugi::xml_node &node, const char *defpath) : Source(node,Source::Kernel,"/boot/x86_64/loader/linux","${kernel-file}") {
		debug("Source for '",name(),"' will be ",url);
		if(!(message && *message)) {
			message = _("Getting installation kernel");
		}
	}

 }
