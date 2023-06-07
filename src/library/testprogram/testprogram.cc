
 #include <udjat/tools/logger.h>
 #include <udjat/tools/quark.h>
 #include <libreinstall/source.h>
 #include <iostream>
 #include <set>

 using namespace std;
 using namespace Udjat;

 int main(int, char **) {

	Udjat::Quark::init();
	Udjat::Logger::redirect();
	Udjat::Logger::enable(Udjat::Logger::Trace);
	Udjat::Logger::enable(Udjat::Logger::Debug);
	Udjat::Logger::console(true);

	Reinstall::Source source{
		"http://localhost/~perry/openSUSE-Leap-15.4-NET-x86_64",
		"/home/perry/Público/openSUSE-Leap-15.4-NET-x86_64",
		"/"
	};

	if(source.local()) {
		cout << "Source is local" << endl;
	} else {
		cout << "Source is remote" << endl;
	}

	std::set<Reinstall::Source::File> files;

	source.prepare(files);


	return 0;
 }
