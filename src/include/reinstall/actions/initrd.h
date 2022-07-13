
 #pragma once

 #include <reinstall/action.h>

 namespace Reinstall {

	/// @brief The linuxrc for the installation.
	class UDJAT_API InitRD : public Action::Source {
	public:
		InitRD(const pugi::xml_node &node) : Source(node,"","/boot/x86_64/loader/initrd") {
		}

	};

 }
