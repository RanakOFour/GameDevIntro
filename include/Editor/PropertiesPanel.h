#ifndef PROPERTIESPANEL_H
#define PROPERTIESPANEL_H

#include "Editor/Editor.h"
#include "Editor/Panel.h"
#include "RanakEngine/RanakEngine.h"

#include <map>

class PropertiesPanel : public Panel
{
private:
	std::map<std::string, std::string> m_stringValueMap;
	std::map<int, std::string> m_entityNameMap;
	bool m_showCategoryMenu;
	int m_selectedEntityId;
	ImVec2 m_position;
	ImVec2 m_size;

	void DrawEntityProperties(int _entityId);
	void DrawCategoryAttributes(const int& _entityId, const std::string& _categoryName, sol::table& _attributes);

public:
	PropertiesPanel(std::weak_ptr<Editor> _editor);
	~PropertiesPanel();

	void Draw();

	void SetPosition(ImVec2 _pos);

	void SetEntity(int _id);
	int GetEntity();
};

#endif