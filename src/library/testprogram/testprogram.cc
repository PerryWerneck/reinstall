
 #include "private.h"
 #include <udjat/tools/logger.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/application.h>
 #include <udjat/agent.h>
 #include <udjat/module.h>

 #include <reinstall/controller.h>
 #include <reinstall/actions/isobuilder.h>
 #include <reinstall/dialogs.h>
 #include <reinstall/writer.h>

 #include <reinstall/actions/kernel.h>
 #include <reinstall/actions/initrd.h>
 #include <reinstall/userinterface.h>

 #include <udjat/moduleinfo.h>

 #include <unistd.h>
 #include <reinstall/diskimage.h>

 using namespace std;
 using namespace Udjat;

 static const Udjat::ModuleInfo moduleinfo{"Net install builder"};

 class TestModule : public Udjat::Module, public Udjat::Factory {
 public:
	TestModule() : Udjat::Module("test", moduleinfo), Udjat::Factory("option",moduleinfo) {
	}

	bool push_back(const pugi::xml_node &node) override {

		class Action : public Reinstall::IsoBuilder {
		private:
			const char *filename;

		public:
			Action(const pugi::xml_node &node) : Reinstall::IsoBuilder(node), filename{getAttribute(node,"filename","/tmp/test.iso")} {

				debug("Creating iso-writer action '",name(),"'");

				if(!(icon_name && *icon_name)) {
					// https://specifications.freedesktop.org/icon-naming-spec/latest/
					// drive-removable-media
					icon_name = "document-save-as";
				}

			}

			virtual ~Action() {
			}

			bool interact() override {
				return filename != nullptr;
			}

			std::shared_ptr<Reinstall::Writer> WriterFactory() override {
				info() << "Saving '" << filename << "'" << endl;
				return Reinstall::Writer::FileFactory(filename);
			};

		};

		Reinstall::push_back(node,make_shared<Action>(node));

		return true;

	}

 };

 int main(int argc, char **argv) {

	setlocale( LC_ALL, "" );
	Udjat::Quark::init(argc,argv);

	// Get default UI
	Reinstall::UserInterface interface;

	// First get controller to construct the factories.
	Reinstall::Controller::getInstance();

	/*
	{
		Reinstall::Disk::Image("efi.iso","vfat").forEach([](const char *mountpoint, const char *path){
			cout << mountpoint << " " << path << endl;
		});
	}
	*/

	{
		// Initialize module.
		new TestModule();

		// Initialize application, load xml definitions.
		Udjat::Application::init(argc,argv,"./test.xml");

		Reinstall::Dialog::Progress progress;

		Reinstall::Action &action = Reinstall::Action::get_selected();

		if(action.interact()) {
			action.prepare()->burn(action.WriterFactory());
		}

	}

	// Finalize application.
	Udjat::Application::finalize();
	return 0;

}
