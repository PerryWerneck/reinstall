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
 #include <reinstall/worker.h>
 #include <reinstall/dialogs.h>
 #include <vector>
 #include <udjat/tools/intl.h>

 namespace Reinstall {

	Worker::Worker() {
	}

	void Worker::apply(Source UDJAT_UNUSED(&source)) {
	}

	void Worker::pre(Action &action) {
	}

	void Worker::post(Action &action) {
	}

	size_t Worker::size() {
		return 0;
	}

	void Worker::burn(std::shared_ptr<Reinstall::Writer> UDJAT_UNUSED(writer)) {
	}

 }

