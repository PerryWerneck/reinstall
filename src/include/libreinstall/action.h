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
 #include <memory>

 #include <libreinstall/source.h>
 #include <libreinstall/builder.h>
 #include <libreinstall/repository.h>

 #include <libreinstall/dialogs/progress.h>
 #include <udjat/tools/object.h>

 namespace Reinstall {

	/// @brief Standard action.
	class UDJAT_API Action : public Udjat::NamedObject {
	private:
		static Action *selected;	/// @brief Selected action.
		static Action *def;			/// @brief Default action.

	protected:

		/// @brief List of repositories defined by XML.
		std::vector<Reinstall::Repository> repositories;

		/// @brief List of sources defined by XML.
		std::vector<std::shared_ptr<Reinstall::Source>> sources;

		/// @brief List of templates defined by XML.
		std::vector<Reinstall::Template> templates;

		/// @brief Get image builder.
		virtual std::shared_ptr<Reinstall::Builder> BuilderFactory() const;

		/// @brief Get image writer.
		virtual std::shared_ptr<Reinstall::Writer> WriterFactory() const;

		/// @brief Get files, apply templates (if required).
		virtual void prepare(Dialog::Progress &progress, std::set<std::shared_ptr<Reinstall::Source::File>> &files) const;

		/// @brief Build iso image.
		virtual void build(Dialog::Progress &progress, std::shared_ptr<Reinstall::Builder> builder, std::set<std::shared_ptr<Reinstall::Source::File>> &files) const;

		/// @brief Build iso image.
		virtual std::shared_ptr<Reinstall::Builder> build(Dialog::Progress &progress, std::set<std::shared_ptr<Reinstall::Source::File>> &files) const;

		/// @brief Write iso image.
		virtual void write(Dialog::Progress &progress, std::shared_ptr<Reinstall::Builder> builder, std::shared_ptr<Reinstall::Writer> writer) const;

		/// @brief Write iso image.
		virtual void write(Dialog::Progress &progress, std::shared_ptr<Reinstall::Builder> builder) const;

		/// @brief The output defined by xml
		const struct OutPut {

			/// @brief The output filename.
			const char * filename = nullptr;

			/// @brief The required file length.
			unsigned long long length = 0;

			constexpr OutPut() = default;
			OutPut(const Udjat::XML::Node &node);

		} output;

	public:

		Action(Action *) = delete;
		Action(Action &) = delete;

		Action() {
		};

		Action(const Udjat::XML::Node &node);
		~Action();

		enum ActivationType {
			Selected,
			Default
		};

		bool getProperty(const char *key, std::string &value) const override;

		static void activate(const ActivationType type = Selected);

		virtual void activate(Dialog::Progress &progress) const;

		inline void select() noexcept {
			selected = this;
		}

	};

 }
