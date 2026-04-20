#ifndef SCENEEDITTAB_H
#define SCENEEDITTAB_H

#include "RanakEngine/RanakEngine.h"

#include "Editor/Editor.h"
#include "Editor/EntityPanel.h"
#include "Editor/CategoryPanel.h"
#include "Editor/RulesPanel.h"
#include "Editor/PropertiesPanel.h"
#include "Editor/CameraPanel.h"
#include "Editor/SceneSettings.h"
#include "Editor/SceneSettingsPanel.h"
#include "Editor/ConsolePanel.h"
#include "Editor/AssetBrowserPanel.h"
#include "Editor/Gizmo.h"

#include "imgui/imgui.h"

#include <SDL3/SDL.h>

#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

/**
 * @class SceneEditTab
 * @brief The main 3D scene-editing tab of the editor.
 *
 * Owns the live RE::Core::Scene, the orthographic Camera, an infinite-grid
 * shader, and all scene editor panels (Entity, Category, Rules, Camera,
 * Properties).  Each frame it renders the OpenGL grid, runs scene Draw(),
 * then draws the ImGui panels on top.  Entity selection is performed via
 * raycasting from HandleInput() in Editor.
 */
class SceneEditTab
{
    friend Editor;
    friend class TopBar;
    
    private:
	Editor& m_editor;                         ///< Back-reference to the owning Editor.
	std::weak_ptr<RE::Core::Scene>  m_scene;  ///< The active scene.
	std::shared_ptr<RE::IO::Window>   m_window; ///< SDL/OpenGL window handle.
	std::shared_ptr<RE::Core::Camera> m_camera; ///< The scene camera.

    std::shared_ptr<RE::Asset::Shader> m_gridShader; ///< Infinite-grid GLSL shader.
    GLuint m_dummyGridVAO = 0; ///< VAO used to issue the grid draw call (no vertex data needed).

    // UI Panels
    EntityPanel       m_entityPanel;      ///< Lists all scene entities.
    CategoryPanel     m_categoryPanel;    ///< Category loading and assignment.
    RulesPanel        m_rulesPanel;       ///< Rule management.
    CameraPanel       m_cameraPanel;      ///< Camera settings.
    PropertiesPanel   m_propertiesPanel;  ///< Selected-entity field editor.
    SceneSettingsPanel m_settingsPanel;   ///< Per-scene physics and render settings.
    ConsolePanel      m_consolePanel;     ///< Log message console.
    AssetBrowserPanel m_assetBrowserPanel; ///< Project file browser.

    SceneSettings m_sceneSettings; ///< Live physics/render settings for this scene.

    int  m_selectedEntityId = -1; ///< ID of the primary selected entity, or -1 for none.
    std::vector<int> m_selectedEntities; ///< All currently selected entity IDs (multi-select).

    bool m_isGameRunning = false; ///< True while the scene simulation is playing.
    bool m_isGamePaused  = false; ///< True while the simulation is paused (running but frozen).
    Uint64 m_lastFrameTime = 0;   ///< SDL performance counter value at the previous frame.
    std::string m_savedSceneState; ///< Serialized scene snapshot taken when Play is pressed.

    bool m_showContext = false; ///< Whether the right-click context menu is showing.

    bool m_isDraggingRect     = false; ///< True while drawing a drag-select rectangle.
    Vector2 m_dragRectStart;             ///< Screen-space start of the drag-select rectangle.
    Vector2 m_dragRectEnd;               ///< Screen-space end of the drag-select rectangle.

    bool m_isDraggingEntity = false; ///< True while dragging the selected entity in the viewport.
    bool m_isDraggingGizmo  = false; ///< True while dragging a gizmo handle.
    bool m_isPanningCamera  = false; ///< True while panning the camera via MMB drag.
    Vector2 m_panAnchorWorld;        ///< World-space point locked to the mouse during MMB pan.
    Gizmo::Axis m_activeGizmoAxis = Gizmo::Axis::None; ///< Gizmo axis being dragged.
    Vector2 m_dragStartWorldPos;     ///< World position of the mouse when drag started.
    Vector2 m_dragStartEntityPos;    ///< Entity position when drag started (for undo).
    Vector2 m_dragStartEntityScale;  ///< Entity scale when drag started (for scale undo).
    float   m_dragStartEntityRot = 0.0f; ///< Entity rotation when drag started (for rotate undo).

    /** @brief Per-entity transform snapshot at drag start, for multi-entity undo. */
    struct EntityDragStart
    {
        Vector2 position;
        Vector2 scale;
        float   rotation;
    };
    std::unordered_map<int, EntityDragStart> m_dragStartTransforms; ///< Per-entity drag start transforms.

    // Parent-child hierarchy (editor-level, not in the engine)
    std::unordered_map<int, int> m_entityParent;              ///< Child ID -> Parent ID.
    std::unordered_map<int, std::vector<int>> m_entityChildren; ///< Parent ID -> Children IDs.

    /** @brief Persistent record of a user-loaded rule, independent of the active scene. */
    struct RuleRecord
    {
        std::string name;     ///< Rule name (derives from the .lua file stem).
        std::string filePath; ///< Absolute path to the source .lua file.
    };

    /// Rules the editor knows about.  Survives scene load and simulation stop/start.
    std::vector<RuleRecord> m_ruleRegistry;

    /** @brief Re-adds every registry rule to the current scene that is not already present. */
    void ReapplyRegistryToScene();

    /** @brief Renders all editor panels and the context menu (disabling them during locked tutorial steps). */
    void DrawEditorUI();
    /** @brief Renders the right-click context menu (create/delete entity, show panels). */
    void DrawContextMenu();

    public:
    /**
     * @brief Constructs the tab, creates the scene, camera, grid shader, and loads the EditorRender rule.
     * @param _editor Reference to the owning Editor.
     */
    SceneEditTab(Editor& _editor);
    ~SceneEditTab();

    /** @brief Renders one complete frame: grid, scene, panels. */
    void Draw();

    /** @brief Starts scene simulation: calls Init on all rules then begins Update each frame. */
    void Run();
    /** @brief Stops scene simulation and resets to the pre-play snapshot. */
    void Stop();
    /** @brief Pauses scene simulation (physics and updates freeze; resume is possible). */
    void Pause();
    /** @brief Resumes a paused simulation. */
    void Resume();

    /**
     * @brief Selects the given entity and shows the PropertiesPanel.
     * @param _id Entity ID to select (-1 to clear selection).
     */
    void SelectEntity(int _id);
    /**
     * @brief Toggles the given entity in the multi-selection.
     * @param _id Entity ID to toggle.
     */
    void ToggleEntitySelection(int _id);
    /**
     * @brief Selects all entities in the current scene.
     */
    void SelectAllEntities();
    /**
     * @brief Clears the entire multi-selection.
     */
    void ClearSelection();
    /**
     * @brief Returns true if the given entity is in the selection.
     * @param _id Entity ID to check.
     */
    bool IsEntitySelected(int _id) const;
    
    /** @brief Returns the ID of the primary selected entity, or -1. */
    int GetSelectedEntity();
    /** @brief Returns all selected entity IDs. */
    const std::vector<int>& GetSelectedEntities() const { return m_selectedEntities; }
    /** @brief Returns true while the scene simulation is playing. */
    bool IsGameRunning() const { return m_isGameRunning; }
    /** @brief Returns true while the simulation is paused. */
    bool IsGamePaused()  const { return m_isGamePaused; }
    /** @brief Returns a reference to the per-scene physics and render settings. */
    SceneSettings& GetSceneSettings() { return m_sceneSettings; }
    /**
     * @brief Looks up a scene-editor panel by its window title.
     * @return Pointer to the matching Panel, or nullptr if not found.
     */
    Panel* GetPanelByName(const std::string& _name);

    /** @brief Returns reference to entity list panel. */
    EntityPanel& GetEntityPanel() { return m_entityPanel; }
    /** @brief Returns reference to properties panel. */
    PropertiesPanel& GetPropertiesPanel() { return m_propertiesPanel; }

    void CloseAllPanels() { m_entityPanel.SetShown(false); m_categoryPanel.SetShown(false); m_rulesPanel.SetShown(false); m_cameraPanel.SetShown(false); m_propertiesPanel.SetShown(false); m_settingsPanel.SetShown(false); m_consolePanel.SetShown(false); m_assetBrowserPanel.SetShown(false); };

    // Parent-child hierarchy

    /**
     * @brief Sets the parent of an entity. Pass -1 to unparent.
     * @param _childId  The entity to reparent.
     * @param _parentId The new parent entity ID, or -1 to make it a root.
     */
    void SetEntityParent(int _childId, int _parentId);
    /**
     * @brief Returns the parent of the given entity, or -1 if root.
     */
    int GetEntityParent(int _entityId) const;
    /**
     * @brief Returns the children of the given entity.
     */
    const std::vector<int>& GetEntityChildren(int _entityId) const;
    /**
     * @brief Returns true if the entity has no parent (is a root entity).
     */
    bool IsRootEntity(int _entityId) const;

    /**
     * @brief Adds a user-loaded rule to the persistent registry.
     * @param _name     Rule name (file stem).
     * @param _filePath Absolute path to the .lua file.
     */
    void RegisterRule(const std::string& _name, const std::string& _filePath);
    /** @brief Clears the rule registry (called before loading a new scene file). */
    void ClearRuleRegistry();
    /**
     * @brief Populates the registry from the rules currently in the scene.
     * Called after a scene file load so the registry reflects the loaded scene.
     */
    void RebuildRegistryFromScene();
    /** @brief Read-only access to the rule registry for display in the Rules panel. */
    const std::vector<RuleRecord>& GetRuleRegistry() const { return m_ruleRegistry; }
};

#endif