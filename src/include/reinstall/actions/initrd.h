
 #pragma once

 #include <reinstall/source.h>

 namespace Reinstall {

	/// @brief The linuxrc for the installation.
	class UDJAT_API InitRD : public Source {
	public:
		InitRD(const pugi::xml_node &node) : Source(node,"","/boot/x86_64/loader/initrd") {
		}

	};

 }
