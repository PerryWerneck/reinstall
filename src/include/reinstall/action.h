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

 #include <reinstall/defs.h>
 #include <pugixml.hpp>
 #include <udjat/tools/object.h>
 #include <udjat/tools/string.h>
 #include <reinstall/object.h>
 #include <reinstall/repository.h>
 #include <reinstall/value.h>
 #include <reinstall/source.h>
 #include <reinstall/dialogs/popup.h>
 #include <list>
 #include <unordered_set>
 #include <memory>
 #include <cstring>
 #include <functional>

 namespace Reinstall {

 	class Worker;

	class UDJAT_API Action : public Udjat::NamedObject {
	public:

		/// @brief Kernel parameters.
		class UDJAT_API KernelParameter {
		private:
			const char * nm = "";
			const char * repository = "install";
			Udjat::String vl;

			enum Type : uint8_t {
				Invalid,
				Value,			///< @brief Standard string value.
				Url
			} type = Invalid;

		public:
			KernelParameter(const pugi::xml_node &node);
			~KernelParameter();

			void set(const Action &action);

			inline const char * name() const noexcept {
				return nm;
			}

			const char * value() const noexcept {
				return vl.c_str();
			}

		};

		/// @brief Template for image contents replacing.
		class UDJAT_API Template {
		private:
			/// @brief Template name.
			const char *name;

			/// @brief Template URL.
			const char *url;

			/// @brief Template file name.
			std::string filename;

		public:
			Template(const pugi::xml_node &node);
			~Template();

			bool test(const char *path) const noexcept;

			void load(const Udjat::Object &object);
			void apply(Source &source);

			/// @brief copy loaded template to file.
			/// @param path Path to filename.
			void replace(const char *path) const;

			inline const char * c_str() const noexcept {
				return name;
			}

		};

		std::list<std::shared_ptr<Template>> templates;

	protected:

		const struct Options {

			bool enabled 	: 1;	/// @brief True if the action is enabled.
			bool visible	: 1;	/// @brief True if the action is visible.
			bool reboot		: 1;	/// @brief True if the action will request reboot.
			bool quit		: 1;	/// @brief True if the action will enable 'quit application' button.

			Options(const pugi::xml_node &node);

			/// @brief Construct option from xml node.
			static bool Factory(const pugi::xml_node &node, const char *attrname, bool def);

		} options;

		/// @brief Output file.
		const char * output_file = nullptr;

		/// @brief Kernel parameters.
		std::vector<KernelParameter> kparms;

		/// @brief Repositories list.
		std::unordered_set<std::shared_ptr<Repository>, Repository::Hash, Repository::Equal> repositories;

		/// @brief Sources list.
		std::unordered_set<std::shared_ptr<Source>, Source::Hash, Source::Equal> sources;

		/// @brief Search for source based on image path
		std::shared_ptr<Source> source(const char *path);

		/// @brief Scan xml for 'tagname', call lambda in every occurrence.
		/// @param tagname the <tag> to search for.
		/// @param node the start node.
		/// @param call The method to call on every tag (returning 'true' stop the search).
		/// @return true if the 'call' has returned true.
		bool scan(const pugi::xml_node &node, const char *tagname, const std::function<bool(const pugi::xml_node &node)> &call);

		/// @brief Run first step with worker.
		virtual void prepare(Reinstall::Worker &worker);

		/// @brief Icon name (for dialogs and menus).
		const char *icon_name = "";

		/// @brief Object with the UI definitions.
		std::shared_ptr<Abstract::Object> item;

		Action(const pugi::xml_node &node, const char *icon_name = "");

	private:
		static Action * selected;		///< @brief Selected action.

		struct {
			Dialog::Popup confirmation;
			Dialog::Popup success;
			Dialog::Popup failed;
		} dialog;

	public:
		virtual ~Action();

		unsigned short id;

		static Action & get_selected();

		inline void set_selected() noexcept {
			selected = this;
		}

		inline bool is_selected() const noexcept {
			return selected == this;
		}

		inline bool enabled() const noexcept {
			return options.enabled;
		}

		inline bool visible() const noexcept {
			return options.visible;
		}

		inline bool reboot() const noexcept {
			return options.reboot;
		}

		inline bool quit() const noexcept {
			return options.quit;
		}

		inline const char * get_icon_name() const noexcept {
			return icon_name;
		}

		/// @brief Get UI Object.
		inline std::shared_ptr<Abstract::Object> get_button() {
			return item;
		}

		inline std::string get_label() const {
			return item->get_label();
		}

		/// @brief Get repository.
		std::shared_ptr<Repository> repository(const char *name = "install") const;

		/// @brief Get parameters from user (first step, gui thread).
		/// @return false to cancel action.
		virtual bool interact();

		/// @brief Create image (second step, work thread).
		/// @return Pointer to action worker.
		virtual std::shared_ptr<Reinstall::Worker> WorkerFactory();

		/// @brief Construct file writer (Runs on main thread).
		virtual std::shared_ptr<Reinstall::Writer> WriterFactory();

		/// @brief Load folders.
		void load();

		/// @brief Apply templates.
		void applyTemplates();

		/// @brief Return the URL for installation media.
		virtual const char * install();

		bool push_back(std::shared_ptr<Source> source);
		bool push_back(std::shared_ptr<Template> tmpl);

		inline size_t source_count() const noexcept {
			return sources.size();
		}

		void for_each(const std::function<void (Source &source)> &call);
		void for_each(const std::function<void (std::shared_ptr<Source> &source)> &call);

		inline const Dialog::Popup & confirmation() const {
			return dialog.confirmation;
		}

		inline const Dialog::Popup & success() const {
			return dialog.success;
		}

		inline const Dialog::Popup & failed() const {
			return dialog.failed;
		}

	};

	UDJAT_API void push_back(const pugi::xml_node &node, std::shared_ptr<Action> action);

 }
