
 #include "private.h"
 #include <udjat/moduleinfo.h>
 #include <reinstall/controller.h>

 namespace Reinstall {

	static const Udjat::ModuleInfo moduleinfo{"BBReinstall group"};

	Controller::Controller() : Udjat::Factory("group",moduleinfo) {
	}

	Controller & Controller::getInstance() {
		static Controller controller;
		return controller;
	}

	std::shared_ptr<Group> Controller::find(const pugi::xml_node &node) {

		const char * name = node.attribute("name").as_string("default");

		for(auto group : groups) {
			if(*group == name) {
				return group;
			}
		}

		auto group = make_shared<Group>(node);
		groups.push_back(group);
		return group;

	}

	bool Controller::push_back(const pugi::xml_node &node) {
		find(node);
		return true;
	}

 }

