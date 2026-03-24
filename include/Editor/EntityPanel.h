#ifndef ENTITY_PANEL_H
#define ENTITY_PANEL_H

#include "RanakEngine/RanakEngine.h"
#include "Editor/Panel.h"

#include <vector>
#include <memory>
#include <optional>

class Editor;

class EntityPanel : public Panel
{
    private:
    std::vector<int> m_cachedEntities;

    // Optional so empty constructor works
    // Reference wrapper because optional does not like type& reference
    std::optional<std::reference_wrapper<RE::Core::EntityRegistry>> m_registry;

    void RefreshEntityList();

    public:
    EntityPanel() : m_registry() {};
    EntityPanel(std::weak_ptr<Editor> _editor);
    ~EntityPanel();

    void Draw();

    void AddEntity();
    void RemoveEntity(int _id);
};

#endif
