#ifndef ENTITY_PANEL_H
#define ENTITY_PANEL_H

#include "RanakEngine/RanakEngine.h"
#include "Editor/Panel.h"

#include <vector>
#include <memory>
#include <map>

class Editor;

class EntityPanel : public Panel
{
    private:

    int m_selectedEntity;
    bool m_showAddToCategoryMenu;

    std::vector<int> m_cachedEntities;
    std::map<std::string, std::string> m_stringValueMap;

    void RefreshEntityList();

    void DrawEntityProperties(int _entityId);
    void DrawCategoryAttributes(const int& _entityId, const std::string& _categoryName, sol::table& _attributes);

    public:
    EntityPanel(std::weak_ptr<Editor> _editor);
    ~EntityPanel();

    void Draw();
    void AddEntity();
    void RemoveEntity(int _id);
    void SelectEntity(int _id);
    int GetSelectedEntity() const { return m_selectedEntity; }
};

#endif
