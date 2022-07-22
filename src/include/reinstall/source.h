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

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/object.h>
 #include <pugixml.hpp>
 #include <memory>
 #include <vector>

 namespace Reinstall {

	class Action;

	/// @brief File/Folder to copy from repository to image.
	class UDJAT_API Source : public Udjat::NamedObject {
	private:
		std::string tempfilename;			///< @brief If not empty, the temporary file name.

	public:
		const char *url;					///< @brief The file URL.
		const char *repository;				///< @brief Repository name.
		const char *path;					///< @brief The path inside the image.
		const char *message;				///< @brief User message while downloading source.
		const char *filename = nullptr;		///< @brief Nome do arquivo local.

		/// @brief Create a simple source.
		Source(const char *name, const char *url, const char *path);

		/// @brief Create new file source.
		/// @param node XML definitions for this file source.
		/// @param url Default URL.
		/// @param defpath Default path.
		Source(const pugi::xml_node &node, const char *url="", const char *defpath="");
		virtual ~Source();

		inline bool operator< (const Source &b) const noexcept {
			return strcasecmp(path,b.path) < 0;
		}

		bool operator> (const Source &b) const noexcept {
			return strcasecmp(path,b.path) > 0;
		}

		bool operator== (const Source &b) const noexcept {
			return strcasecmp(path,b.path) == 0;
		}

		/// @brief Download file.
		/// @return Nome do arquivo local.
		std::string save();

		/// @brief Get folders contents.
		/// @param action The current action.
		/// @param contents Vector for the folder contents.
		/// @return true if the source is a folder.
		virtual bool contents(const Action &action, std::vector<std::shared_ptr<Source>> &contents);

		typedef struct {
			bool operator() (const std::shared_ptr<Source> a, const std::shared_ptr<Source> b) const {
				return strcmp(a->path,b->path) == 0;
			}
		} Equal;

		typedef struct {
			size_t operator() (const std::shared_ptr<Source> a) const {
				size_t rc = 5381;
				for (const signed char *p = (const signed char *) a->path; *p != '\0'; p++)
					rc = (rc << 5) + rc + *p;
				return rc;
			}
		} Hash;

	};

 }

