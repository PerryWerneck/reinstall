
 #include "private.h"
 #include <udjat/tools/logger.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/application.h>
 #include <udjat/agent.h>
 #include <udjat/module.h>

 #include <reinstall/controller.h>
 #include <reinstall/actions/isobuilder.h>
 #include <reinstall/dialogs.h>

 #include <reinstall/actions/kernel.h>
 #include <reinstall/actions/initrd.h>

 #include <udjat/moduleinfo.h>

 #include <unistd.h>

 using namespace std;
 using namespace Udjat;

 static const Udjat::ModuleInfo moduleinfo{"Net install builder"};

 class TestModule : public Udjat::Module, public Udjat::Factory {
 public:
	TestModule() : Udjat::Module("test", moduleinfo), Udjat::Factory("option",moduleinfo) {
	}

	bool push_back(const pugi::xml_node &node) override {

		class Action : public Reinstall::IsoBuilder {
		public:
			Action(const pugi::xml_node &node) : Reinstall::IsoBuilder(node) {
			}

			virtual ~Action() {
			}

			void write(Reinstall::iso9660::Worker &worker) override {
				worker.save("/tmp/test.iso");
				patch("/tmp/test.iso");
			}

		};

		Reinstall::Group::find(node)->push_back(new Action(node));

		return true;
	}

 };

 int main(int argc, char **argv) {

	setlocale( LC_ALL, "" );

	Udjat::Quark::init(argc,argv);

	// First get controller to construct the factories.
	Reinstall::Controller::getInstance();

	// Initialize module.
	new TestModule();

	// Initialize application, load xml definitions.
	Udjat::Application::init(argc,argv,"./test.xml");

	Reinstall::Action::getDefault().activate();

	// Finalize application.
	Udjat::Application::finalize();
	return 0;

}
