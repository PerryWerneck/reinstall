/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2022 Perry Werneck <perry.werneck@gmail.com>
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

 #include <config.h>
 #include <udjat/module.h>
 #include <udjat/factory.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <reinstall/action.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/protocol.h>
 #include <iostream>
 #include <reinstall/dialogs/progress.h>
 #include <reinstall/builder.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>

 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 using namespace std;
 using namespace Udjat;

 Udjat::Module * udjat_module_init() {

 	static const Udjat::ModuleInfo moduleinfo { "IsoWriter module" };

	class Module : public Udjat::Module, public Udjat::Factory {
	public:
		Module() : Udjat::Module("IsoWriter", moduleinfo), Udjat::Factory("iso-writer",moduleinfo) {
		}

		bool push_back(const pugi::xml_node &node) override {

			class Action : public Reinstall::Action {
			private:
				const char *url;

			public:
				Action(const pugi::xml_node &node) : Reinstall::Action(node,"drive-removable-media"), url{getAttribute(node,"url","")} {

					if(!(url && *url)) {
						throw runtime_error(_("Required attribute 'URL' is missing"));
					}

				}

				virtual ~Action() {
				}

				std::shared_ptr<Reinstall::Builder> WorkerFactory() override {

					sleep(1);

					// Download image.
					std::string filename = Udjat::Application::CacheDir("iso").build_filename(url);
					debug("Cache filename is ",filename.c_str());

					Reinstall::Dialog::Progress &progress = Reinstall::Dialog::Progress::getInstance();
					auto worker = Protocol::WorkerFactory(this->url);

					progress.set_sub_title(_("Downloading ISO image"));
					progress.set_url(worker->url().c_str());

					worker->save(filename.c_str(),[&progress](double current, double total){
						progress.set_progress(current,total);
						return true;
					});

					int fd = ::open(filename.c_str(),O_RDONLY);
					if(fd < 0) {
						throw system_error(errno, system_category(), _("Cant access downloaded image"));
					}

					class Worker : public Reinstall::Builder {
					private:
						int fd;

					public:
						Worker(int f) : fd{f} {
						}

						virtual ~Worker() {
							::close(fd);
						}

						size_t size() override {
							struct stat statbuf;
							if(fstat(fd,&statbuf) != 0) {
								cerr << "isowriter\tCan't get image file size: " << strerror(errno) << endl;
								return 0;
							}
							return statbuf.st_size;
						}

						void burn(std::shared_ptr<Reinstall::Writer> writer) override {

							Reinstall::Dialog::Progress &progress = Reinstall::Dialog::Progress::getInstance();
							progress.set_sub_title(_("Writing image"));

							size_t current = 0;
							size_t total = size();

							#define BUFLEN 2048
							unsigned char buffer[BUFLEN];

							while(current < total) {

								size_t length = (total - current);
								if(length > BUFLEN) {
									length = BUFLEN;
								}

								ssize_t bytes = ::read(fd, buffer, length);
								if(bytes < 0) {
									throw system_error(errno,system_category(),_("Cant read from image file"));
								}

								if(bytes == 0) {
									throw runtime_error(_("Unexpected EOF reading image file"));
								}

								writer->write(buffer,length);

								current += length;
								progress.set_progress(current,total);

							}

							progress.set_sub_title(_("Finalizing"));
							writer->finalize();
							writer->close();

							progress.set_sub_title(_(""));

						}

					};

					return make_shared<Worker>(fd);

				}

			};

			Reinstall::push_back(node,make_shared<Action>(node));

			return true;
		}

	};

	return new Module();

 }

