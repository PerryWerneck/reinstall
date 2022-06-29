
 #include "private.h"
 #include <udjat/tools/logger.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/application.h>
 #include <udjat/agent.h>
 #include <udjat/module.h>

 #include <reinstall/controller.h>
 #include <reinstall/action.h>
 #include <reinstall/dialogs.h>

 #include <reinstall/actions/kernel.h>
 #include <reinstall/actions/initrd.h>
 #include <reinstall/iso9660.h>

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

		class Action : public Reinstall::Action {
		public:
			Action(const pugi::xml_node &node) : Reinstall::Action(node) {

				// Get URL for installation kernel.
				if(!scan(node,"kernel",[this](const pugi::xml_node &node) {
					push_back(make_shared<Reinstall::Kernel>(node));
					return true;
				})) {
					throw runtime_error("Missing required entry <kernel> with the URL for installation kernel");
				}

				// Get URL for installation init.
				if(!scan(node,"init",[this](const pugi::xml_node &node) {
					push_back(make_shared<Reinstall::InitRD>(node));
					return true;
				})) {
					throw runtime_error("Missing required entry <init> with the URL for the linuxrc program");
				}

			}

			virtual ~Action() {
			}

			void activate() {

				cout << "***************************" << __FILE__ << "(" << __LINE__ << ")" << endl;
				Reinstall::Dialog::Progress progress(*this);
				cout << "***************************" << __FILE__ << "(" << __LINE__ << ")" << endl;
				Reinstall::iso9660::Worker worker;
				cout << "***************************" << __FILE__ << "(" << __LINE__ << ")" << endl;
				Reinstall::Action::activate(worker);
				cout << "***************************" << __FILE__ << "(" << __LINE__ << ")" << endl;
				//worker.save("/tmp/test.iso");
				cout << "***************************" << __FILE__ << "(" << __LINE__ << ")" << endl;

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
