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
 #include <pugixml.hpp>
 #include <reinstall/object.h>
 #include <list>
 #include <unordered_set>
 #include <memory>
 #include <cstring>
 #include <functional>

 namespace Reinstall {

 	class Worker;

	class UDJAT_API Action : public Reinstall::Object {
	public:
		/// @brief File/Folder to copy from repository to image.
		class UDJAT_API Source {
		private:
			std::string tempfilename;			///< @brief If not empty, the temporary file name.

		public:
			const char *url;					///< @brief The file URL.
			const char *path;					///< @brief The path inside the image.
			const char *message;				///< @brief User message while downloading file.
			const char *filename = nullptr;		///< @brief Nome do arquivo local.

			/// @brief Create a simple source.
			Source(const char *url, const char *path);

			/// @brief Create new file source.
			/// @param node XML definitions for this file source.
			/// @param url Default URL.
			/// @param defpath Default path.
			Source(const pugi::xml_node &node, const char *url="", const char *defpath="");

			~Source();

			/// @brief Check if it's required to download the source.
			inline bool local() const noexcept {
				return (*path != 0);
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

			/// @brief Download file.
			/// @return Nome do arquivo local.
			std::string save();

		};


		typedef struct {
			bool operator() (const std::shared_ptr<Source> a, const std::shared_ptr<Source> b) const {
				return strcmp(a->path,b->path) == 0;
			}
		} SourceEqual;

		typedef struct {
			size_t operator() (const std::shared_ptr<Source> a) const {
				size_t rc = 5381;
				for (const signed char *p = (const signed char *) a->path; *p != '\0'; p++)
					rc = (rc << 5) + rc + *p;
				return rc;
			}
		} SourceHash;

	protected:

		std::unordered_set<std::shared_ptr<Source>, SourceHash, SourceEqual> sources;

		/// @brief Scan xml for 'tagname', call lambda in every occurrence.
		/// @param tagname the <tag> to search for.
		/// @param node the start node.
		/// @param call The method to call on every tag (returning 'true' stop the search).
		/// @return true if the 'call' has returned true.
		bool scan(const pugi::xml_node &node, const char *tagname, const std::function<bool(const pugi::xml_node &node)> &call);

		/// @brief Activate with worker.
		virtual void activate(Reinstall::Worker &worker);

		/// @brief Get first folder.
		std::shared_ptr<Source> folder();

	private:
		static Action * defaction;		///< @brief Default action.

	public:
		Action(const pugi::xml_node &node);
		virtual ~Action();

		unsigned short id;

		static Action & getDefault();

		virtual void activate();

		/// @brief Load folders.
		void load();

		/// @brief Return the URL for installation media.
		virtual const char * install();

		bool push_back(std::shared_ptr<Source> source);

		void for_each(const std::function<void (Source &source)> &call);
		void for_each(const std::function<void (std::shared_ptr<Source> &source)> &call);

	};

 }
