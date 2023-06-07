
 #include <udjat/tools/logger.h>
 #include <udjat/tools/quark.h>

 using namespace std;
 using namespace Udjat;

 int main(int argc, char **argv) {

	Udjat::Quark::init();
	Udjat::Logger::redirect();
	Udjat::Logger::enable(Udjat::Logger::Trace);
	Udjat::Logger::enable(Udjat::Logger::Debug);
	Udjat::Logger::console(true);


	return 0;
 }
