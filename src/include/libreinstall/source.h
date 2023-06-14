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
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <list>
 #include <memory>
 #include <string>
 #include <set>
 #include <libreinstall/slpclient.h>

 namespace Reinstall {

	/// @brief Common data source.
	class UDJAT_API Source : public Udjat::NamedObject {
	private:

		/// @brief Source paths.
		const struct Path {

			/// @brief Remote path.
			const char * remote = "";

			/// @brief Local path.
			const char * local = "";

			constexpr Path(const char *r, const char *l) : remote{r}, local{l} {
			}

			Path(const Udjat::XML::Node &node);

		} path;

		/// @brief SLP settings for automatic source detection.
		const SlpClient slpclient;

	protected:

		/// @brief Path in the target image.
		const char *imgpath = "";

	public:

		/// @brief Build Source using fixed values.
		/// @param remote URL of the root for remote source.
		/// @param local Path for source in the local file system.
		/// @param path Path for source in the target image.
		constexpr Source(const char *n, const char *remote, const char *local = "", const char *path = "") : Udjat::NamedObject{n}, path{remote,local}, imgpath{path} {
		}

		Source(const Udjat::XML::Node &node);

		/// @brief Get local path (if available).
		/// @return The source path in local filesystem or nullptr if not available.
		virtual const char * local() const noexcept;

		/// @brief Get remote path, resolve SLP if necessary.
		/// @return The remote source path.
		virtual const char * remote() const;

		/// @brief File to write.
		class UDJAT_API File : public std::string {
		public:

			/// @brief Construct file from string.
			/// @param imgpath Path for file in the target image.
			File(const std::string &imgpath) : std::string{imgpath} {
			}

			/// @brief Construct file from string.
			/// @param imgpath Path for file in the target image.
			File(const char *imgpath) : std::string{imgpath} {
			}

			/// @brief Is the file remote?
			virtual bool remote() const noexcept = 0;

			/// @brief Path of the file in local file system.
			virtual const char * path() const = 0;

			/// @brief Save file using custom writer.
			/// @param writer The file writer method.
			virtual void save(const std::function<void(unsigned long long offset, unsigned long long total, const void *buf, size_t length)> &writer) const = 0;

		};

		/// @brief Get list of source files.
		virtual void prepare(std::set<std::shared_ptr<File>> &files) const;

	};

 }

