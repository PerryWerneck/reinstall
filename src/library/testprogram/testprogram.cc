
 #include <udjat/defs.h>
 #include <udjat/tools/logger.h>

 #include <unistd.h>

 using namespace std;
 using namespace Udjat;

 int main(int argc, char **argv) {

	setlocale( LC_ALL, "" );

	Logger::redirect(nullptr,true);

	Udjat::load("./test.xml");



	Module::unload();

	return 0;
}
