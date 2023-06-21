
 #include <udjat/tools/logger.h>
 #include <udjat/tools/quark.h>
 #include <libreinstall/source.h>
 #include <libreinstall/iso9660.h>
 #include <udjat/module.h>
 #include <iostream>
 #include <set>
 #include <vector>
 #include <reinstall/userinterface.h>
 #include <udjat/ui/dialog.h>
 #include <udjat/tools/logger.h>
 #include <pugixml.hpp>

 #include <libreinstall/builder.h>
 #include <libreinstall/builders/iso9660.h>
 #include <libreinstall/builders/fat.h>

 #include <libreinstall/action.h>

 #include <libreinstall/writers/file.h>

 #include <libreinstall/sources/zip.h>

 using namespace std;
 using namespace Udjat;

 /*
 static void object_test() {

	class Action : public Reinstall::Action {
	public:
		Action() : Reinstall::Action{} {

			sources.emplace_back(
				"suse",
				"http://localhost/~perry/openSUSE-Leap-15.4-NET-x86_64/",
				"/home/perry/Público/openSUSE-Leap-15.4-NET-x86_64",
				""
			);

			tmpls.emplace_back(
				"grub",
				"grub.cfg",
				"file://../../templates/grub.cfg"
			);
		}

		std::shared_ptr<Reinstall::Builder> BuilderFactory() const override {
			static iso9660::Settings settings;
			return iso9660::BuilderFactory(settings);
		}

		std::shared_ptr<Reinstall::Writer> WriterFactory() const override {
			return make_shared<Reinstall::FileWriter>("/tmp/test.iso");
		}

	};

	Reinstall::Dialog::Progress progress;
	Action{}.activate(progress);

 }
 */

 static void xml_test(const char *filename) {

	class Action : public Reinstall::Action {
	public:
		Action(const Udjat::XML::Node &node) : Reinstall::Action{node} {
		}

		std::shared_ptr<Reinstall::Builder> BuilderFactory() const override {
			static iso9660::Settings settings;
			return iso9660::BuilderFactory(settings);
		}

		std::shared_ptr<Reinstall::Writer> WriterFactory() const override {
			return make_shared<Reinstall::FileWriter>("/tmp/test.iso");
		}

	};

	pugi::xml_document document;
	document.load_file(filename);

	Action{document.document_element()}.activate();

 }

 int main(int, char **) {

	Udjat::Quark::init();
	Udjat::Logger::redirect();
	Udjat::Logger::enable(Udjat::Logger::Trace);
	Udjat::Logger::enable(Udjat::Logger::Debug);
	Udjat::Logger::console(true);
	Udjat::Module::load("http");

	Dialog::Controller dialogs;

	xml_test("test.xml");

	Udjat::Module::unload();
	return 0;
 }
