#ifndef PROPERTIESPANEL_H
#define PROPERTIESPANEL_H

#include "Editor/Panel.h"
#include "RanakEngine/RanakEngine.h"

#include "imgui/imgui.h"

#include <map>
#include <optional>

class PropertiesPanel : public Panel
{
	private:
	std::map<std::string, std::string> m_stringValueMap;
	std::map<int, std::string> m_entityNameMap;

	bool m_showAddToCategory;

	bool m_setPosition;
	ImVec2 m_position;
	ImVec2 m_size;

	std::optional<std::reference_wrapper<RE::Core::EntityRegistry>> m_registry;

	void DrawEntityProperties(int _entityId);
	void DrawCategoryAttributes(int _entityId, std::string _categoryName, sol::table _attributes);

	public:
	PropertiesPanel() {};
	PropertiesPanel(std::weak_ptr<Editor> _editor);
	~PropertiesPanel();

	void Draw();

	void SetPosition(ImVec2 _pos);
	ImVec2 GetSize();
};

#endif