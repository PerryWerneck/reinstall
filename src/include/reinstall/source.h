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

 /*
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
	protected:
		struct {
			std::string temp;			///< @brief If not empty, the temporary file name.
			std::string saved;			///< @brief The filename used to download.
		} filenames;

	public:

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

#ifndef _WIN32
		/// @brief Extract mountpoint from path.
		/// @param path The file path to extract.
		/// @return String with path without the device mountpoint.
		static const std::string fspath(const char *path);
#endif // _WIN32

		/// @brief Create a simple source.
		/// @param url Default URL.
		/// @param path The default image path.
		Source(const char *name, const char *url, const char *path);

		/// @brief Create new file source.
		/// @param node XML definitions for this file source.
		/// @param url Default URL.
		/// @param defpath Default path.
		Source(const pugi::xml_node &node, const Type type=Common, const char *url="", const char *defpath="");

		virtual ~Source();

		inline bool saved() const noexcept {
			return !filenames.saved.empty();
		}

		const char *filename(bool rw = false);

		/// @brief Get path relative to partition.
		const char * rpath() const;

		inline void set_filename(const char *filename) noexcept {
			filenames.saved = filename;
		}

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

		/// @brief Download to temporary file.
		void save();

		/// @brief Download to defined file.
		virtual void save(const char *filename);

		/// @brief Save file with custom writer.
		virtual void save(const std::function<void(const void *buf, size_t length)> &write);

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
 */

