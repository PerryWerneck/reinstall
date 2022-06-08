
 #pragma once

 #include <pugixml.hpp>
 #include <udjat/defs.h>
 #include <udjat/tools/object.h>

 namespace Reinstall {

	class UDJAT_API Worker {
	public:

		Worker(const pugi::xml_node &node);

		virtual void pre();
		virtual void apply();
		virtual void post();

	};

 }
