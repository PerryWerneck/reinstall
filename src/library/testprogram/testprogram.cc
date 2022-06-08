
 #include <udjat/defs.h>
 #include <udjat/tools/logger.h>

 #include <reinstall/controller.h>

 #include <unistd.h>

 using namespace std;
 using namespace Udjat;

 int main(int argc, char **argv) {

	setlocale( LC_ALL, "" );

	Reinstall::Controller::getInstance();


	return 0;
}
