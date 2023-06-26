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

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/ui/dialog.h>

 namespace Udjat {

	/// @brief Abstract progress dialog.
	class UDJAT_API Dialog::Popup {
	private:
		bool running = true;

	protected:
		Popup(const Popup &) = delete;
		Popup(const Popup *) = delete;

	public:
		Popup();
		virtual ~Popup();

		/// @brief Setup from dialog.
		virtual void set(const Dialog &dialog);

		/// @brief is the dialog running?
		inline bool enabled() const noexcept {
			return running;
		}

		inline operator bool() const noexcept {
			return running;
		}

		/// @brief Enable/Disable button
		/// @param id The button id.
		/// @param enabled the button enable status.
		virtual void enable(int id, bool enabled = true);

		inline void disable(int id) {
			enable(id,false);
		}

	};

 }
