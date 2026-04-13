#ifndef EDITOR_H
#define EDITOR_H

#include "RanakEngine/RanakEngine.h"

#include "Editor/Project.h"
#include "Editor/StateRegistry.h"
#include "Editor/TutorialPanel.h"
#include "Editor/UndoManager.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_opengl3.h"

#include <memory>
#include <vector>
#include <string>

class SceneEditTab;
class TextEditTab;

/**
 * @class Editor
 * @brief Top-level application object owning the engine, ImGui context, and all editor tabs.
 *
 * Must be created via Editor::Create() (factory pattern enforced so that
 * shared_from_this() is valid during construction).  Drives the main loop
 * in Run(), calling Draw() each frame which dispatches to SceneEditTab or
 * TextEditTab depending on the active tab index.  The menu bar and tutorial
 * panel are owned here so they persist across tab switches.
 */
class Editor
{
public:
    enum State
    {
        SceneEdit,
        TextEdit
    };
    
private:
    std::shared_ptr<SceneEditTab> m_sceneEdit; ///< The 2D scene-editing tab.
    std::shared_ptr<TextEditTab>  m_textEdit;  ///< The Lua source-file editing tab.
    ImFont* m_font;                            ///< Custom font loaded for the editor UI.

    RE::EngineContents m_engineContents; ///< Aggregated engine systems.
    Project            m_project;        ///< The currently open project (path + subdir helpers).

    State m_state; ///< The current editor state (active tab).

    StateRegistry m_stateRegistry; ///< Named conditions and actions registered by all editor subsystems.

    bool m_showLoadDialog = false;   ///< True when the "Load Category" file dialog is open.
    bool m_showSaveDialog = false;   ///< True when the "Create Category" dialog is open.
    bool m_showSettingsDialog = false; ///< True when the Project Settings modal should be drawn.

    std::string m_currentScenePath; ///< Absolute path to the currently loaded scene file, or empty.

    TutorialPanel m_tutorialPanel; ///< In-editor guided tutorial overlay (universal across tabs).
    UndoManager m_undoManager;      ///< Undo/redo command stack.

    /** @brief Initialises Dear ImGui (context, style, SDL3/OpenGL backends). */
    void InitImGui();
    /** @brief Tears down Dear ImGui and releases its resources. */
    void CleanupImGui();
    /** @brief Polls SDL events and forwards them to ImGui and the scene camera. */
    void HandleInput();
    /** @brief Renders the application menu bar (universal across tabs). */
    void DrawMenuBar();
    /** @brief Renders the Project Settings modal dialog (opened from the menu bar). */
    void DrawSettingsDialog();
    /** @brief Applies the current ProjectSettings to the live engine subsystems. */
    void ApplyProjectSettings();
    /** @brief Writes ProjectInfo.lua at the project root with the current editor state. */
    void SaveProjectInfo();
    /** @brief Reads ProjectInfo.lua from the project root and restores editor state. */
    void LoadProjectInfo();

    Editor(RE::EngineContents engineContents, Project project);
    public:
    ~Editor();

    /**
     * @brief Factory method — thin wrapper returning a prvalue for guaranteed copy elision.
     * @param engineContents  Pre-initialised engine bundle (window, GL, etc.).
     * @param project         The project selected on the project-selection screen.
     * @return The new Editor constructed directly in the caller's frame (C++17 RVO).
     */
    static Editor Create(RE::EngineContents engineContents, Project project)
    {
        return Editor(std::move(engineContents), std::move(project));
    }

    /** @brief Enters the main loop; returns when the user closes the editor. */
    void Run();
    /** @brief Renders one complete frame (clear, ImGui, swap). */
    void Draw();

    /** @brief Returns a reference to the engine sub-system bundle. */
    RE::EngineContents& GetEngineContents() { return m_engineContents; }
    /** @brief Returns a reference to the current project. */
    Project& GetProject() { return m_project; }
    /** @brief Returns a reference to the SceneEditTab. */
    SceneEditTab& GetSceneEdit();
    /** @brief Returns a reference to the tutorial panel. */
    TutorialPanel& GetTutorialPanel() { return m_tutorialPanel; }
    /** @brief Returns the current editor state (active tab). */
    State GetState() const { return m_state; }
    /** @brief Switches between the scene-edit tab and text-edit tab. */
    void SetState(State _state) { m_state = _state; }
    /** @brief Returns a reference to the named condition/action registry. */
    StateRegistry& GetStateRegistry() { return m_stateRegistry; }
    /** @brief Returns a reference to the undo/redo manager. */
    UndoManager& GetUndoManager() { return m_undoManager; }
};

#endif