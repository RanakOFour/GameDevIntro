#ifndef SCENEEDITTAB_H
#define SCENEEDITTAB_H

#include "RanakEngine/RanakEngine.h"

#include "Editor/Editor.h"
#include "Editor/EntityPanel.h"
#include "Editor/CategoryPanel.h"
#include "Editor/RulesPanel.h"
#include "Editor/PropertiesPanel.h"
#include "Editor/CameraPanel.h"
#include "Editor/TutorialPanel.h"

#include "imgui/imgui.h"

#include <memory>
#include <vector>
#include <string>

/**
 * @class SceneEditTab
 * @brief The main 3D scene-editing tab of the editor.
 *
 * Owns the live RE::Core::Scene, the orthographic Camera, an infinite-grid
 * shader, and all editor panels (Entity, Category, Rules, Camera, Properties,
 * Tutorial).  Each frame it renders the OpenGL grid, runs scene Draw(), then
 * draws the menu bar and all ImGui panels on top.  Entity selection is
 * performed via raycasting from HandleInput() in Editor.
 */
class SceneEditTab
{
    friend Editor;
private:
	std::weak_ptr<Editor> m_editor;           ///< Back-reference to the owning Editor.
	std::shared_ptr<RE::IO::Window>   m_window; ///< SDL/OpenGL window handle.
	std::shared_ptr<RE::Core::Scene>  m_scene;  ///< The active scene.
	std::shared_ptr<RE::Core::Camera> m_camera; ///< The scene camera.

    std::shared_ptr<RE::Asset::Shader> m_gridShader; ///< Infinite-grid GLSL shader.
    GLuint m_dummyGridVAO = 0; ///< VAO used to issue the grid draw call (no vertex data needed).

    // UI Panels
    EntityPanel     m_entityPanel;     ///< Lists all scene entities.
    CategoryPanel   m_categoryPanel;   ///< Category loading and assignment.
    RulesPanel      m_rulesPanel;      ///< Rule management.
    CameraPanel     m_cameraPanel;     ///< Camera settings.
    PropertiesPanel m_propertiesPanel; ///< Selected-entity field editor.
    TutorialPanel   m_tutorialPanel;   ///< In-editor guided tutorial overlay.

    int  m_selectedEntityId = -1; ///< ID of the currently selected entity, or -1 for none.

    bool m_isEditorRunning = true;  ///< Set to false by the File > Exit menu item.
    bool m_isGameRunning   = false; ///< True while the scene simulation is playing.

    bool m_showContext = false; ///< Whether the right-click context menu is showing.

    /** @brief Renders all editor panels and the context menu (disabling them during locked tutorial steps). */
    void DrawEditorUI();
    /** @brief Renders the right-click context menu (create/delete entity, show panels). */
    void DrawContextMenu();
    /** @brief Handles keyboard shortcuts. */
    void HandleInput();

public:
    /**
     * @brief Constructs the tab, creates the scene, camera, grid shader, and loads the EditorRender rule.
     * @param _editor Weak pointer to the owning Editor.
     */
    SceneEditTab(std::weak_ptr<Editor> _editor);
    ~SceneEditTab();

    /** @brief Renders one complete frame: grid, scene, menu bar, panels, tutorial overlay. */
    void Draw();
    /**
     * @brief Renders the application menu bar.
     *
     * Exposed as public so Editor::Draw() can call it regardless of which tab is active,
     * giving the menu bar a universal presence.
     */
    void DrawMenuBar();

    /** @brief Starts (or resumes) scene simulation. */
    void Run();

    /**
     * @brief Selects the given entity and shows the PropertiesPanel.
     * @param _id Entity ID to select.
     */
    void SelectEntity(int _id);
    /** @brief Returns the ID of the currently selected entity, or -1. */
    int GetSelectedEntity();
};

#endif