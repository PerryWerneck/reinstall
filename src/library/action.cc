
 #include "private.h"
 #include <reinstall/action.h>

 namespace Reinstall {

	Action::Action(const pugi::xml_node &node) : Object(node) {
	}

	void Action::pre() {
		for(auto worker : workers) {
			worker->pre();
		}
	}

	void Action::apply() {
		for(auto worker : workers) {
			worker->apply();
		}
	}

	void Action::post() {
		for(auto worker : workers) {
			worker->post();
		}
	}

 }

