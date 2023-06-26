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
 #include <libreinstall/kernelparameter.h>

 #include <udjat/ui/dialog.h>
 #include <udjat/ui/menu.h>
 #include <udjat/ui/dialogs/progress.h>
 #include <udjat/tools/object.h>

 using Progress = Udjat::Dialog::Progress;

 namespace Reinstall {

	/// @brief Standard action.
	class UDJAT_API Action : public Udjat::NamedObject, private Udjat::Menu::Item {
	private:
		static Action *selected;	/// @brief Selected action.
		static Action *def;			/// @brief Default action.

		/// @brief Action popup
		class Popup : public Udjat::Dialog {
		private:
			bool allow_reboot = false;
			bool allow_quit = false;

		public:
			Popup(const char *name, const Udjat::XML::Node &node) {
				Udjat::Dialog::setup(name,node);
			}

			void setup(const Udjat::XML::Node &node) override;

			void run(const char *error_message, bool allow_close = true) const;
			void run(bool allow_close = true) const;
		};

		/// @brief Action dialogs.
		struct Dialogs {

			/// @brief Confirmation dialog (Yes/No/Quit)
			Udjat::Dialog confirmation;

			/// @brief Progress dialog.
			Udjat::Dialog progress;

			/// @brief Success dialog.
			Popup success;

			/// @brief Success dialog.
			Popup failed;

			Dialogs(const Udjat::XML::Node &node) :
				confirmation{"confirmation",node},
				progress{"progress",node},
				success{"success",node},
				failed{"failed",node} {
			}

		} dialogs;

	protected:

		/// @brief List of repositories defined by XML.
		std::vector<Reinstall::Repository> repositories;

		/// @brief Get repository by name
		const Repository & repository(const char *name) const;

		/// @brief List of sources defined by XML.
		std::vector<std::shared_ptr<Reinstall::Source>> sources;

		/// @brief List of templates defined by XML.
		std::vector<Reinstall::Template> templates;

		/// @brief List of kernel parameters defined by XML.
		std::vector<std::shared_ptr<Reinstall::Kernel::Parameter>> kparms;

		/// @brief Get image builder.
		virtual std::shared_ptr<Reinstall::Builder> BuilderFactory() const;

		/// @brief Get image writer.
		virtual std::shared_ptr<Reinstall::Writer> WriterFactory() const;

		/// @brief Get files, apply templates (if required).
		virtual void prepare(Progress &progress, Source::Files &files) const;

		/// @brief Build iso image.
		virtual void build(Progress &progress, std::shared_ptr<Reinstall::Builder> builder, Source::Files &files) const;

		/// @brief Write iso image.
		virtual void write(Progress &progress, std::shared_ptr<Reinstall::Builder> builder, std::shared_ptr<Reinstall::Writer> writer) const;

		/// @brief The output defined by xml
		const struct OutPut {

			/// @brief The output filename.
			const char * filename = nullptr;

			/// @brief The required file length.
			unsigned long long length = 0;

			constexpr OutPut() = default;
			OutPut(const Udjat::XML::Node &node);

		} output;

		void reboot() noexcept;

	public:

		Action(Action *) = delete;
		Action(Action &) = delete;

		Action(const Udjat::XML::Node &node);
		~Action();

		enum ActivationType {
			Selected,
			Default
		};

		bool getProperty(const char *key, std::string &value) const override;

		static void activate(const ActivationType type);

		void activate() override;

		inline void select() noexcept {
			selected = this;
		}

	};

	void UDJAT_API push_back(std::shared_ptr<Action> action);

 }
