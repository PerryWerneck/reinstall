
 #pragma once
 #include <reinstall/group.h>
 #include <list>
 #include <memory>
 #include <udjat/factory.h>

 namespace Reinstall {

	class UDJAT_API Controller : Udjat::Factory {
	private:

		/// @brief Lista de grupos registrados.
		std::list<std::shared_ptr<Group>> groups;

		/// @brief Último grupo incluído (default).
		std::shared_ptr<Group> group;

		Controller();

	public:

		static Controller & getInstance();

		/// @brief Find group, create and insert if not exist.
		std::shared_ptr<Group> find(const pugi::xml_node &node);

		/// @brief Factory a new group from XML node.
		/// @param XML definition for the new element.
		/// @return true if the node whas inserted.
		bool push_back(const pugi::xml_node &node) override;

	};

 }
