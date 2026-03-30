#ifndef ENTITY_PANEL_H
#define ENTITY_PANEL_H

#include "RanakEngine/RanakEngine.h"
#include "Editor/Panel.h"

#include <vector>
#include <memory>
#include <optional>

class Editor;

/**
 * @class EntityPanel
 * @brief Editor panel listing all entities present in the current scene.
 *
 * Maintains a cached copy of the entity ID list that refreshes each frame.
 * Allows the user to select an entity (highlighting it in the PropertiesPanel)
 * as well as creating or deleting entities.
 */
class EntityPanel : public Panel
{
    private:
    std::vector<int> m_cachedEntities; ///< Entity IDs cached from the registry each frame.

    // Optional so empty constructor works
    // Reference wrapper because optional does not like type& reference
    std::optional<std::reference_wrapper<RE::Core::EntityRegistry>> m_registry; ///< Ref to the scene's entity registry.

    /** @brief Renders the entity list window. */
    void Draw() override;

    public:
    EntityPanel() : m_registry() {};
    /**
     * @brief Constructs the panel and caches the entity registry reference.
     * @param _editor Weak pointer to the owning Editor.
     */
    EntityPanel(std::weak_ptr<Editor> _editor);
    ~EntityPanel();

    /** @brief Adds a new empty entity to the scene and selects it. */
    void AddEntity();
    /**
     * @brief Removes the entity with the given ID from the scene.
     * @param _id ID of the entity to remove.
     */
    void RemoveEntity(int _id);

    /** @brief Refreshes m_cachedEntities from the live EntityRegistry. */
    void RefreshEntityList();
};

#endif
