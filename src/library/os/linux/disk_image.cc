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

/*
 #include <bbreinstall/disk.h>
 #include <bbreinstall/activity.h>
 #include <cstdlib>
 #include <sys/mount.h>
 #include <cstring>
 #include <system_error>
 #include <unistd.h>
 #include <linux/loop.h>
 #include <fcntl.h>
 #include <iostream>
 #include <sys/types.h>
 #include <dirent.h>
 #include <sys/stat.h>
 #include <sys/types.h>

 using namespace std;

 namespace Reinstall {

	Disk::Image::Image(const char *filename, const char *filesystemtype) {

		//
		// Cria diretório temporário
		//
		mountpoint = Activity::mkdtemp();

		//
		// Monta imagem no diretório
		//
		// https://stackoverflow.com/questions/11295154/how-do-i-loop-mount-programmatically
		//

		try {

			//
			// Obtém dispositivo /dev/loop disponível
			//
			fd.loopctl = open("/dev/loop-control", O_RDWR);
			if(fd.loopctl == -1) {
				cerr << "Erro ao abrir '/dev/loop-control'" << endl;
				throw system_error(errno, system_category(),filename);
			}

			devnr = ioctl(fd.loopctl, LOOP_CTL_GET_FREE);

			if (devnr == -1) {
				close(fd.loopctl);
				throw system_error(errno, system_category(),filename);
			}

			loopname = "/dev/loop";
			loopname += to_string(devnr);

			fd.loop = open(loopname.c_str(), O_RDWR);
			if(fd.loop == -1) {
				close(fd.loop);
				cerr << "Erro ao abrir '" << loopname << "'" << endl;
				throw system_error(errno, system_category(),filename);
			}

			//
			// Associa dispositivo loop com a imagem
			//
			fd.image = open(filename, O_RDWR);
			if (fd.image == -1) {
				close(fd.loop);
				throw system_error(errno, system_category(),filename);
			}

			if (ioctl(fd.loop, LOOP_SET_FD, fd.image) == -1) {
				close(fd.loop);
				close(fd.image);
				throw system_error(errno, system_category(),filename);
			}

			//
			// Configura dispositivo para limpeza automática.
			//
			struct loop_info loopinfo;
			memset(&loopinfo,0,sizeof(loopinfo));

			if (ioctl(fd.loop, LOOP_GET_STATUS, &loopinfo) == -1) {
				close(fd.loop);
				close(fd.image);
				throw system_error(errno, system_category(),filename);
			}

			loopinfo.lo_flags |= LO_FLAGS_AUTOCLEAR;

			if (ioctl(fd.loop, LOOP_SET_STATUS, &loopinfo) == -1) {
				cerr << "Error '" << strerror(errno) << "' ao ativar 'Auto clear' em " << loopname << endl;
			} else {
				cout << "'Auto clear' foi ativado no dispositivo " << loopname << endl;
			}

			//
			// Monta
			//
			if(mount(loopname.c_str(), mountpoint.c_str(), filesystemtype, MS_SYNCHRONOUS, "") == -1) {
				close(fd.loop);
				close(fd.image);
				cerr << "Erro ao mountar '" << filename << "' usando '" << loopname << "'" << endl;
				throw system_error(errno, system_category(),filename);
			}

#ifdef DEBUG
			cout << "Imagem " << filename << " montada em " << loopname << endl;
#endif // DEBUG

		} catch(...) {
			rmdir(mountpoint.c_str());
			throw;
		}

	}

	static void retry(std::function<bool(int current, int total)> exec) {

		for(size_t ix = 0; ix < 10; ix++) {

			if(exec(ix+1,10)) {
#ifdef DEBUG
				cout << "Encerrando na tentativa " << ix+1 << endl;
#endif // DEBUG
				return;
			}

			usleep(100);

		}

		cerr << "Número máximo de tentativas atingido" << endl;

	}

	Disk::Image::~Image() {

#ifdef DEBUG
			cout << "Imagem '" << mountpoint << "' desmontada de '" << loopname << "'" << endl;
#endif // DEBUG

		retry([this](int current, int total){

			if(umount2(mountpoint.c_str(),MNT_FORCE)) {
				cout << "Dispositivo '" << loopname << "' desmontado com sucesso na tentativa nº " << current << endl;
				return true;
			}

			cerr	<< "Erro ao desmontar '" << loopname << "': " << strerror(errno)
					<< " (tentativa " << current << " de " << total << ")"
					<< endl;

			return false;

		});

		retry([this](int current, int total){

			if (ioctl(fd.loop, LOOP_CLR_FD, 0) == 0) {
				cout << "Dispositivo '" << loopname << "' liberado com sucesso na tentativa nº " << current << endl;
				return true;
			}

			cerr	<< "Erro ao liberar '" << loopname << "': " << strerror(errno)
					<< " (tentativa " << current << " de " << total << ")"
					<< endl;

			return false;

		});

		// Libera o dispositivo.
		close(fd.loop);
		close(fd.image);

		retry([this](int current, int total){

			if(ioctl(fd.loopctl, LOOP_CTL_REMOVE, devnr) != -1) {
				cout << "Dispositivo '" << loopname << "' removido com sucesso na tentativa nº " << current << endl;
				return true;
			}

			cerr	<< "Erro ao remover '" << loopname << "': " << strerror(errno)
					<< " (tentativa " << current << " de " << total << ")"
					<< endl;

			return false;

		});

		close(fd.loopctl);

		// Remove o diretório.
		rmdir(mountpoint.c_str());
	}

	/// @brief Executa função em todos os arquivos da imagem.
	void Disk::Image::forEach(std::function<void (const std::string &mountpoint, const std::string &dirname, const char *basename)> call) {

		DIR *dir = opendir(mountpoint.c_str());
		if(!dir) {
			cerr << "Can't open '" << mountpoint << "'" << endl;
			throw system_error(errno, system_category(),"Can't open mountpoint");
		}

		try {

			for(struct dirent *entry = readdir(dir); entry; entry = readdir(dir)) {

				if(entry->d_name[0] == '.')
					continue;


				if(entry->d_type == DT_DIR) {

					forEach(entry->d_name,call);

				} else {

					call(mountpoint,"/",entry->d_name);
				}


			}

		} catch(...) {

			closedir(dir);
			throw;

		}

		closedir(dir);

	}

	/// @brief Executa função a partir de um diretporio da imagem.
	void Disk::Image::forEach(const char *dirname, std::function<void (const std::string &mountpoint, const std::string &dirname, const char *basename)> call) {

		DIR *dir = opendir( (mountpoint + "/" + dirname).c_str());
		if(!dir) {
			throw system_error(errno, system_category(),"Can't open image path");
		}

		try {

			for(struct dirent *entry = readdir(dir); entry; entry = readdir(dir)) {

				if(entry->d_name[0] == '.')
					continue;

				if(entry->d_type == DT_DIR) {

					forEach( (string(dirname) + "/" + entry->d_name).c_str(),call);

				} else {

					call(mountpoint,dirname,entry->d_name);

				}

			}

		} catch(...) {

			closedir(dir);
			throw;

		}

		closedir(dir);


	}

 }
 
*/
