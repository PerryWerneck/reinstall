
 #include <udjat/tools/logger.h>
 #include <udjat/tools/quark.h>
 #include <libreinstall/source.h>
 #include <libreinstall/builder.h>
 #include <udjat/module.h>
 #include <iostream>
 #include <set>
 #include <reinstall/userinterface.h>
 #include <reinstall/dialogs/progress.h>
 #include <udjat/tools/logger.h>

 #include <libreinstall/sources/zip.h>

 using namespace std;
 using namespace Udjat;

 int main(int, char **) {

	Reinstall::UserInterface interface;

	Udjat::Quark::init();
	Udjat::Logger::redirect();
	Udjat::Logger::enable(Udjat::Logger::Trace);
	Udjat::Logger::enable(Udjat::Logger::Debug);
	Udjat::Logger::console(true);
	Udjat::Module::load("http");

	/*
	Reinstall::Source source{
		"suse",
		"http://localhost/~perry/openSUSE-Leap-15.4-NET-x86_64/",
		"/home/perry/Público/openSUSE-Leap-15.4-NET-x86_64",
		""
	};
	*/

	Reinstall::ZipSource source{
		"zip",
		"http://localhost/~perry/win/ziptest.zip",
		"/home/perry/public_html/win/ziptest.zip",
		""
	};

	Reinstall::Dialog::Progress progress;

	std::set<std::shared_ptr<Reinstall::Source::File>> files;

	source.prepare(files);

	debug("Number of source files: ",files.size());

	try {

		auto builder = Reinstall::Builder::fat(20000000000ULL);

		builder->pre();
		for(auto file : files) {
			builder->push_back(file);
		}
		builder->post();

	} catch(const std::exception &e) {

		Logger::String{e.what()}.error("builder");
		Udjat::Module::unload();
		return -1;

	}

	Udjat::Module::unload();
	return 0;
 }
