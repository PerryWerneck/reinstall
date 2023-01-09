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
 #include <udjat/tools/string.h>
 #include <reinstall/defs.h>
 #include <udjat/tools/object.h>
 #include <pugixml.hpp>
 #include <memory>
 #include <vector>

 namespace Reinstall {

	/// @brief File/Folder to copy from repository to image.
	class UDJAT_API Source : public Udjat::NamedObject {
	private:
		struct {
			std::string temp;			///< @brief If not empty, the temporary file name.
			Udjat::String defined;		///< @brief The filename set from xml definition.
			std::string saved;			///< @brief Tem path for saved file (empty if not downloaded).
		} filenames;

	public:

		/// @brief Get cached file name.
		inline const char * local_file() const noexcept {
			return filenames.saved.c_str();
		}

		inline void local_file(const char *filename) noexcept {
			filenames.saved = filename;
		}

		enum Type {
			Common,	///< @brief Common source (file or folder).
			Kernel,	///< @brief Installation kernel.
			InitRD,	///< @brief InitRD image.
			DUD,	///< @brief Driver installation disk image.
		} type = Common;

		const char *url = nullptr;			///< @brief The file URL.
		const char *repository = nullptr;	///< @brief Repository name.
		const char *path = nullptr;			///< @brief The path inside the image.
		const char *message = nullptr;		///< @brief User message while downloading source.
		//const char *filename = nullptr;		///< @brief Local filename.

		/// @brief Create a simple source.
		/// @param url Default URL.
		/// @param defpath path.
		Source(const char *name, const char *url, const char *path);

		/// @brief Create new file source.
		/// @param node XML definitions for this file source.
		/// @param url Default URL.
		/// @param defpath Default path.
		Source(const pugi::xml_node &node, const Type type=Common, const char *url="", const char *defpath="");

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

		bool operator== (const Type b) const noexcept {
			return type == b;
		}

		/// @brief Process source based on action properties.
		/// @param action The current action.
		void set(const Reinstall::Action &object);

		/// @brief Download file, update filename.
		virtual void save();

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

