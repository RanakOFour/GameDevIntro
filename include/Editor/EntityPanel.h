#ifndef ENTITY_PANEL_H
#define ENTITY_PANEL_H

#include "RanakEngine/RanakEngine.h"
#include "Editor/Panel.h"
#include <vector>
#include <memory>

class Editor;

class EntityPanel : public Panel
{
    private:

    int m_selectedEntity;
    std::vector<int> m_cachedEntities;

    void RefreshEntityList();

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
