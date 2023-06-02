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

 #include <config.h>
 #include <reinstall/repository.h>
 #include <udjat/tools/quark.h>
 #include <reinstall/dialogs/progress.h>
 #include <udjat/tools/logger.h>
 #include <udjat/net/ip/address.h>
 #include <udjat/tools/intl.h>
 #include <list>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netdb.h>

 #ifdef HAVE_LIBSLP
	#include <slp.h>
 #endif // HAVE_LIBSLP

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	Repository::SlpClient::SlpClient(const pugi::xml_node &node)
		: service_type{XML::QuarkFactory(node,"slp-service-type").c_str()},
			scope{XML::QuarkFactory(node,"slp-scope").c_str()},
			filter{XML::QuarkFactory(node,"slp-filter").c_str()},
			kparm{XML::QuarkFactory(node,"slp-kernel-parameter").c_str()},
			message{XML::QuarkFactory(node,"slp-search-message").c_str()},
			allow_local{XML::StringFactory(node,"slp-allow-local").as_bool(false)} {
	}

 #ifdef HAVE_LIBSLP

	struct SRV_URL_CB_INFO  {
		SLPError callbackerr;
		list<String> responses;
	};

	static SLPBoolean slpcallback( SLPHandle hslp, const char* srvurl, unsigned short lifetime, SLPError errcode, void *cookie ) {

		// http://www.openslp.org/doc/html/ProgrammersGuide/SLPFindSrvs.html

		if(errcode == SLP_OK || errcode == SLP_LAST_CALL) {

			if(srvurl && *srvurl) {
				Logger::String{"Got response '",srvurl,"'"}.trace("slp");
				((SRV_URL_CB_INFO *) cookie)->responses.emplace_back(srvurl);
			}

			*(SLPError*)cookie = SLP_OK;

		} else {

			Logger::String{"Got error '",errcode,"' on query"}.error("slp");
			*(SLPError*)cookie = errcode;

		}

		return SLP_TRUE;

	}

	static void extract_url_from_response(const std::string &from, std::string &to) {

		const char *ptr = from.c_str();
		for(size_t ix = 0; ix < 2; ix++) {
			ptr = strchr(ptr,':');
			if(!ptr) {
				Logger::String{"Rejecting bad formatted response '",from,"'"}.warning("slp");
			}
			ptr++;
		}

		to.assign(ptr);

	}

	const char * Repository::SlpClient::get_url() {

		if(!url.empty()) {
			return url.c_str();
		}

		debug("-----------------------------------------------------------");

		// Detect URL
		Dialog::Progress &progress = Dialog::Progress::getInstance();

		std::string dialog_sub_title{progress.get_sub_title()};

		progress.set_sub_title((message && *message) ? message : _("Searching for server"));

		progress.set_url(service_type);
		progress.pulse();

		// https://github.com/ManageIQ/slp/blob/master/examples/raw_example.c
		// https://docs.oracle.com/cd/E19455-01/806-0628/6j9vie80v/index.html
		SLPError err;
		SLPHandle hSlp;

		err = SLPOpen(NULL, SLP_FALSE, &hSlp);
		if(err != SLP_OK) {
			Logger::String{"SLPOpen has failed"}.warning("slpclient");
			return "";
		}

		SRV_URL_CB_INFO cbinfo;


		Logger::String{"Searching for ",service_type}.info("slpclient");
		err = SLPFindSrvs(
					hSlp,
					service_type,
					scope,
					filter,
					slpcallback,
					&cbinfo
				);

		// Check prefixes
		size_t prefix_length = strlen(service_type);

		cbinfo.responses.remove_if([this,prefix_length](String &url){

			const char *str = url.c_str();
			if(strncmp(str,service_type,prefix_length)) {
				Logger::String{"Ignoring invalid response '",url,"'"}.warning("slp");
				return true;
			}

			str += prefix_length;
			if(*str != ':') {
				Logger::String{"Ignoring unexpected response '",url,"'"}.warning("slp");
				return true;
			}

			Logger::String{"Accepting valid response '",url,"'"}.trace("slp");

			return false;
		});

		if(cbinfo.responses.empty()) {

			Logger::String{"No valid SLP response for ",service_type}.warning("slpclient");

		} else {

			Logger::String{cbinfo.responses.size()," SLP response(s) for ",service_type}.info("slpclient");

			if(err != SLP_OK) {

				Logger::String{"SLPFindSrvs has failed"}.warning("slpclient");

			} else if(!allow_local) {

				// Ignore local addresses.
				vector<IP::Addresses> addresses; ///< @brief IP Addresses.
				IP::for_each([&addresses](const IP::Addresses &addr){
					addresses.push_back(addr);
					return false;
				});

				for(auto url=cbinfo.responses.begin(); url != cbinfo.responses.end() && this->url.empty(); url++) {

					SLPSrvURL *parsedurl = NULL;
					if(SLPParseSrvURL(url->c_str(),&parsedurl) != SLP_OK) {

						Logger::String{"Cant parse ",url->c_str()}.error("slpclient");

					} else if(parsedurl->s_pcHost && *parsedurl->s_pcHost) {

						struct addrinfo *ai;
						struct addrinfo hints;

						memset(&hints,0,sizeof(hints));
						hints.ai_family   = AF_UNSPEC;
						hints.ai_socktype = SOCK_STREAM;
						hints.ai_protocol = 0;
						hints.ai_flags    = AI_NUMERICSERV;

						string port;
						if(parsedurl->s_iPort) {
							port = std::to_string(parsedurl->s_iPort);
						}

						if(getaddrinfo(parsedurl->s_pcHost, port.c_str(), &hints, &ai)) {

							Logger::String{"Error resolving ",parsedurl->s_pcHost}.error("slpclient");

						} else {

							for(auto rp = ai;rp != NULL && this->url.empty(); rp = rp->ai_next) {

								sockaddr_storage addr{IP::Factory(rp->ai_addr)};
								bool remote = true;

								for(IP::Addresses &local : addresses) {
									if(local.address == addr) {
										remote = false;
										Logger::String {"Ignoring response ",std::to_string(addr)}.trace("slpclient");
									}
								}

								if(remote) {
									extract_url_from_response(*url, this->url);
								}
							}

							freeaddrinfo(ai);
						}

						SLPFree(parsedurl);

					} else {

						Logger::String{"Ignoring response '",*url,"'"}.info("slp");

					}

				}


			} else {

				// Get first address.
				for(String &response : cbinfo.responses) {
					SLPSrvURL *parsedurl = NULL;
					if(SLPParseSrvURL(response.c_str(),&parsedurl) != SLP_OK) {

						Logger::String{"Cant parse ",response.c_str()}.error("slp");

					} else {

						extract_url_from_response(response, this->url);

						SLPFree(parsedurl);

						if(!this->url.empty()) {
							break;
						}
					}

				}
//				this->url = *cbinfo.responses.begin();

			}


		}

		SLPClose(hSlp);

		progress.set_sub_title(dialog_sub_title.c_str());
		progress.set_url(url.c_str());

		return url.c_str();
	}

 #else

	const char * Repository::SlpClient::get_url() {
		return "";
	}

 #endif // HAVE_LIBSLP

 }

