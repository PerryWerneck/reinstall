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
  * @brief Implement ZIP source.
  *
  */

 #include <config.h>
 #include <libreinstall/source.h>
 #include <libreinstall/sources/zip.h>
 #include <libreinstall/dialogs/progress.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/application.h>
 #include <memory>
 #include <thread>
 #include <udjat/tools/protocol.h>
 #include <zip.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	void ZipSource::prepare(std::set<std::shared_ptr<Source::File>> &files) const {

		class Container : public mutex {
		public:
			zip_t *handler = nullptr;

			constexpr Container() = default;

			Container(const char *name) : handler{zip_open(name,ZIP_RDONLY,NULL)} {
				if(!handler) {
					throw runtime_error(string("Cant open '") + name + "'");
				}
			}

			~Container() {
				zip_close(handler);
			}

		};

		//
		// Get ZIPFILE
		//
		shared_ptr<Container> container;

		Dialog::Progress &progress = Dialog::Progress::getInstance();

		const char *local = this->local();
		if(local && *local) {

			// It's a local file, just open it.
			container = make_shared<Container>(local);

		} else {

			// TODO: Allow an optional 'cache' attribute, when false, open zip as a temporary unnamed file.

			// It's a remote file, download to temporary file and open it.
			std::string url{remote()};

			auto worker = Protocol::WorkerFactory(url.c_str());

			Logger::String{"Getting file from ",worker->url().c_str()}.write(Logger::Debug,name());
			progress.set_url(worker->url().c_str());

			if(local && *local) {

				// Have local path, use it!
				worker->save(local,[&progress](double current, double total) {
					progress.set_progress(current,total);
					return true;
				},true);

				container = make_shared<Container>(local);

			} else {

				// No local path, use cache
				std::string filename{Application::CacheDir{}.build_filename(worker->url().c_str())};
				trace("Updating zip file {}",filename);

				worker->save(filename.c_str(),[&progress](double current, double total) {
					progress.set_progress(current,total);
					return true;
				},true);

				container = make_shared<Container>(filename.c_str());

			}

		}

		//
		// Get ZIPFILE contents.
		//

		/// @brief File in zip container.
		class ZipFile : public Source::File {
		private:
			const char *name;
			shared_ptr<Container> zip;	///< @brief zip container.
			struct zip_stat stat;

		public:

			ZipFile(const char *n, shared_ptr<Container> z, const struct zip_stat &sb, const std::string &to) : Source::File{to}, name{n}, zip{z}, stat{sb} {
			}

			/// @brief zip file should be exported to temporary as the remote ones.
			bool remote() const noexcept override {
				return true;
			}

			const char * path() const override {
				throw runtime_error("Unable to get path for zip source");
			}

			void save(const std::function<void(unsigned long long offset, unsigned long long total, const void *buf, size_t length)> &writer) const override {

				{
					string url{"zip://"};
					url += stat.name;

					Logger::String{"Unpacking '",stat.name,"'"}.write(Logger::Debug,name);
					Dialog::Progress::getInstance().set_url(url.c_str());
				}

				lock_guard<mutex> lock(*zip);

				zip_file *zf = zip_fopen_index(zip->handler, stat.index, 0);

				try {

					size_t offset = 0;
					char buffer[4096];
					while (offset != stat.size) {
						auto bufferlength = zip_fread(zf, buffer, 4096);
						writer(offset,stat.size,buffer,bufferlength);
						offset += bufferlength;
					}

				} catch(...) {
					zip_fclose(zf);
					throw;
				}

				zip_fclose(zf);

			}

		};

		// https://gist.github.com/sdasgup3/a0255ebce3e3eec03e6878b47c8c7059
		lock_guard<mutex> lock(*container);

		auto entries = zip_get_num_entries(container->handler,0);
		for(zip_int64_t entry = 0; entry < entries; entry++) {

			struct zip_stat sb;

			if (zip_stat_index(container->handler, entry, 0, &sb) != 0 || *(sb.name+strlen(sb.name)-1) == '/')  {
				continue;
			}

			debug(entry," - ",sb.index," - ",sb.name);

			files.insert(make_shared<ZipFile>(name(),container,sb,sb.name));

		}


	}

 }

