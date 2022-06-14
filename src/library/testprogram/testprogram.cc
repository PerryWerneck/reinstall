
 #include "private.h"
 #include <udjat/tools/logger.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/application.h>
 #include <udjat/agent.h>
 #include <udjat/module.h>

 #include <reinstall/controller.h>
 #include <reinstall/action.h>

 #include <unistd.h>

 using namespace std;
 using namespace Udjat;

 int main(int argc, char **argv) {

	setlocale( LC_ALL, "" );

	Udjat::Quark::init(argc,argv);

	// First get controller to construct the factories.
	Reinstall::Controller::getInstance();

	// Initialize module.
	udjat_module_init();

	// Initialize application, load xml definitions.
	Udjat::Application::init(argc,argv,"./test.xml");

	Reinstall::Action::getDefault().activate();

	// Finalize application.
	Udjat::Application::finalize();
	return 0;

}
