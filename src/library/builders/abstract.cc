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

 /**
  * @brief Brief description of this source.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <libreinstall/builder.h>
 #include <udjat/tools/logger.h>

 using namespace Udjat;

 namespace Reinstall {

	std::shared_ptr<Builder> Builder::factory() {

		class DummyBuilder : public Builder {
		public:

			constexpr DummyBuilder() : Builder{"builder"} {
			}

			void pre() override {
			}

			void post() override {
			}

			void push_back(std::shared_ptr<Reinstall::Source::File> file) override {
				debug("Pushing '",file->c_str(),"' to dummy builder");
			}

			void write(std::shared_ptr<Writer> writer) {
			}

		};

	}

	void Builder::push_back(const Udjat::Abstract::Object &, const std::vector<Reinstall::Template> &) {
	}

 }
