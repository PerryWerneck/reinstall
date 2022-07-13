
 #pragma once

 #include <reinstall/action.h>

 namespace Reinstall {

	/// @brief The source for the installation kernel;
	class UDJAT_API Kernel : public Action::Source {
	public:
		Kernel(const pugi::xml_node &node) : Source(node,"","/boot/x86_64/loader/initrd") {
		}

	};

 }
