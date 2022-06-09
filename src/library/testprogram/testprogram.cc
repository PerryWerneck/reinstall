
 #include "private.h"
 #include <udjat/tools/logger.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/application.h>
 #include <udjat/agent.h>
 #include <udjat/module.h>

 #include <reinstall/controller.h>

 #include <unistd.h>

 using namespace std;
 using namespace Udjat;

 int main(int argc, char **argv) {

	setlocale( LC_ALL, "" );

	// First get controller to create factories.
	Reinstall::Controller &controller = Reinstall::Controller::getInstance();

	// Initialize module.
	udjat_module_init();

	// Initialize application, load xml definitions.
	Udjat::Application::init(argc,argv,"./test.xml");

	cout << endl << "Selecione a imagem de sistema" << endl << endl;

	controller.for_each([](std::shared_ptr<Reinstall::Group> group){

		cout << "\t" << group->id << " - " << group->label << endl;

		return false;
	});

	// Finalize application.
	Udjat::Application::finalize();
	return 0;

}
