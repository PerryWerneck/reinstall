
 #include "private.h"
 #include <udjat/tools/quark.h>
 #include <udjat/tools/string.h>

 namespace Reinstall {

	Object::Object(const pugi::xml_node &node) :
		Udjat::NamedObject(node),
		subtitle("sub-title",node),
		help("help-button",node) {

		label = getAttribute(node,"label",label);
		title = getAttribute(node,"title",title);

	}

	Object::Link::Link(const char *tagname, const pugi::xml_node &node) {

		const pugi::xml_node & child = node.child(tagname);
		if(!child) {
			return;
		}

		// Obtém grupo do arquivo de configuração para os defaults.
		const char *group = child.attribute("settings-from").as_string("link-defaults");

		url = getAttribute(child,group,"url",url);
		label = getAttribute(child,group,"label",label);
		icon_name = getAttribute(child,group,"icon",icon_name);
		tooltip = getAttribute(child,group,"tooltip",tooltip);

	}

	Object::Text::Text(const char *tag, const pugi::xml_node &node) {

		auto text_node = node.child(tag);
		if(!text_node) {
			// TODO: Upsearch
			return;
		}

		// Obtém grupo do arquivo de configuração para os defaults.
		// const char *group = node.attribute("settings-from").as_string("message-defaults");

		body = Quark(Udjat::String(text_node.child_value()).expand(text_node)).c_str();

	}

	Object::Message::Message(const char *tag, const pugi::xml_node &node) : Text(tag,node) {

		auto child = node.child(tag);
		if(!child) {
			return;
		}

		// Obtém grupo do arquivo de configuração para os defaults.
		const char *group = child.attribute("settings-from").as_string("message-defaults");

		title = getAttribute(child,group,"title",title);
		notify = getAttribute(child,group,"notify",false);

	}

 }

