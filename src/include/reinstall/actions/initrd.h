
 #pragma once

 #include <config.h>
 #include <reinstall/object.h>
 #include <reinstall/group.h>
 #include <reinstall/action.h>
 #include <iostream>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	/// @brief The linuxrc for the installation.
	class UDJAT_PRIVATE InitRD : public Action::Source {
	public:
		InitRD(const pugi::xml_node &node) : Source(node,"/boot/x86_64/loader/initrd") {
		}

	};

 }
