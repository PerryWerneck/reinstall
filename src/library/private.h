
 #pragma once

 #include <config.h>
 #include <reinstall/object.h>
 #include <reinstall/group.h>
 #include <reinstall/action.h>
 #include <iostream>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	/// @brief The source for the installation kernel;
	class UDJAT_PRIVATE Kernel : public Source {
	public:
		Kernel(const pugi::xml_node &node) : Source(node) {
		}

	};

	/// @brief The linuxrc for the installation.
	class UDJAT_PRIVATE InitRD : public Source {
	public:
		InitRD(const pugi::xml_node &node) : Source(node) {
		}

	};

 }
