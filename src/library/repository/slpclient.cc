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
 #include <vector>
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
		: service_type{Quark(node,"slp-service-type").c_str()},
			scope{Quark(node,"slp-scope").c_str()},
			filter{Quark(node,"slp-filter").c_str()},
			kparm{Quark(node,"slp-kernel-parameter","").c_str()},
			message{Quark(node,"slp-search-message","").c_str()},
			allow_local{node.attribute("allow-local").as_bool(false)} {
	}

 #ifdef HAVE_LIBSLP

	struct SRV_URL_CB_INFO  {
		SLPError callbackerr;
		vector<string> urls;
	};

	static SLPBoolean slpcallback( SLPHandle hslp, const char* srvurl, unsigned short lifetime, SLPError errcode, void *cookie ) {

		if(srvurl && *srvurl) {

			const char *ptr = strstr(srvurl,"http");
			if(!ptr) {
				Logger::String{"Invalid URL from SLP: '",srvurl,"'"}.warning("slpclient");
			}

			SRV_URL_CB_INFO * info = (SRV_URL_CB_INFO *) cookie;

			Logger::String{"Got ",ptr," from SLP service"}.trace("slpclient");

			info->urls.emplace_back(ptr);

		}

		return SLP_TRUE;

	}

	const char * Repository::SlpClient::get_url() {

		if(!url.empty()) {
			return url.c_str();
		}

		debug("-----------------------------------------------------------");

		// Detect URL
		Dialog::Progress &progress = Dialog::Progress::getInstance();

		if(message && *message) {
			progress.set_sub_title(message);
		}

		progress.set_url(service_type);

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

		if(cbinfo.urls.empty()) {
			Logger::String{"No SLP response for ",service_type}.info("slpclient");
		} else {
			Logger::String{cbinfo.urls.size()," SLP response(s) for ",service_type}.info("slpclient");
		}

		if(err != SLP_OK) {

			Logger::String{"SLPFindSrvs has failed"}.warning("slpclient");

		} else if(!allow_local) {

			// Ignore local addresses.
			vector<IP::Addresses> addresses; ///< @brief IP Addresses.
			IP::for_each([&addresses](const IP::Addresses &addr){
				addresses.push_back(addr);
				return false;
			});

			for(auto url=cbinfo.urls.begin(); url != cbinfo.urls.end() && this->url.empty(); url++) {

				SLPSrvURL *parsedurl = NULL;
				if(SLPParseSrvURL(url->c_str(),&parsedurl) != SLP_OK) {

					Logger::String{"Cant parse ",url->c_str()}.error("slpclient");

				} else {

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
								this->url = url->c_str();
							}
						}

						freeaddrinfo(ai);
					}

					SLPFree(parsedurl);

				}

			}


		} else {

			// Get first address.
			this->url = *cbinfo.urls.begin();

		}

		SLPClose(hSlp);

		return url.c_str();
	}

 #else

	const char * Repository::SlpClient::get_url() {
		return "";
	}

 #endif // HAVE_LIBSLP

 }

