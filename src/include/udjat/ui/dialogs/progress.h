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
	class UDJAT_API Dialog::Progress {
	private:
		static Progress *current;
		Progress *parent = nullptr;

	protected:
		Progress(const Progress &) = delete;
		Progress(const Progress *) = delete;

	public:
		Progress();
		virtual ~Progress();

		/// @brief Setup from dialog.
		virtual void set(const Dialog &dialog);

		/// @brief Show dialog
		virtual void show();

		/// @brief Hide dialog.
		virtual void hide();

		/// @brief Get active progress dialog.
		static Progress & instance();

		/// @brief Set windows title.
		virtual void title(const char *title);

		/// @brief Set message.
		virtual void message(const char *subtitle);

		/// @brief Get message.
		virtual std::string message() const;

		/// @brief Set secondary text.
		//virtual void secondary(const char *step);

		virtual void icon(const char *icon_name);

		virtual void url(const char *url);

		virtual void pulse();
		virtual void progress(double current, double total);
		virtual void count(size_t current, size_t total);

		/// @brief Show progress dialog, run background task.
		/// @return Return code of the background task.
		// virtual void run(const std::function<void(Progress &progress)> &background_task);

		/// @brief Run background task with dialog in 'pulse' mode.
		/// @param message The message for the progress dialog.
		/// @param background_task The task to run.
		// void run(const char *message, const std::function<void()> &background_task);

	};

 }
