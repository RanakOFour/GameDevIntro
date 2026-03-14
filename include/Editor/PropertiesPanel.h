#ifndef PROPERTIES_PANEL_H
#define PROPERTIES_PANEL_H

#include "RanakEngine/RanakEngine.h"
#include <memory>
#include <string>

class Editor;

class PropertiesPanel
{
    private:
    std::weak_ptr<Editor> m_editor;
    bool m_showPanel;
    int m_displayedEntityId;

    void DrawEntityProperties(int _entityId);
    void DrawCategoryAttributes(const std::string& _categoryName, sol::table& _attributes);

    public:
    PropertiesPanel(std::weak_ptr<Editor> _editor);
    ~PropertiesPanel();

    void Draw();
    void SetDisplayedEntity(int _entityId) { m_displayedEntityId = _entityId; }
};

#endif
