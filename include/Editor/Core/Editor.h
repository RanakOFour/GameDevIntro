#ifndef EDITOR_H
#define EDITOR_H

#include "RanakEngine/RanakEngine.h"

#include "Editor/Core/Project.h"
#include "Editor/Core/StateRegistry.h"
#include "Editor/Core/UndoManager.h"
#include "Editor/Panels/UI/TutorialPanel.h"
#include "Editor/Panels/UI/TopBar.h"
#include "Editor/UI/ThemeManager.h"
#include "Editor/Panels/UI/ThemeSettingsPanel.h"

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

    StateRegistry m_stateRegistry; ///< Named conditions and actions registered by all editor subsystems.

    TopBar         m_topBar;        ///< The main menu bar.
    ThemeManager   m_themeManager;  ///< Colour theme preset manager.
    ThemeSettingsPanel m_themeSettingsPanel; ///< Theme colour editing panel.
    ImFont* m_font;                            ///< Custom font loaded for the editor UI.

    sol::table m_editorTable;
    RE::EngineContents m_engineContents; ///< Aggregated engine systems.
    Project            m_project;        ///< The currently open project (path + subdir helpers).

    State m_state; ///< The current editor state (active tab).

    bool m_showSettingsDialog = false; ///< True when the Project Settings modal should be drawn.

    std::string m_currentScenePath; ///< Absolute path to the currently loaded scene file, or empty.

    TutorialPanel m_tutorialPanel; ///< In-editor guided tutorial overlay (universal across tabs).
    UndoManager m_undoManager;      ///< Undo/redo command stack.

    /** @brief Snapshot of a single entity's categories and field values for clipboard. */
    struct EntitySnapshot
    {
        std::string name;
        std::vector<std::pair<std::string, std::vector<std::pair<std::string, sol::object>>>> categories;
    };
    std::vector<EntitySnapshot> m_clipboard; ///< Copy buffer for copy/paste.

    std::string m_savedLayouts[3]; ///< In-memory layout presets.

    /** @brief Initialises Dear ImGui (context, style, SDL3/OpenGL backends). */
    void InitImGui();
    /** @brief Tears down Dear ImGui and releases its resources. */
    void CleanupImGui();
    /** @brief Polls SDL events and forwards them to ImGui and the scene camera. */
    void HandleInput();
    /** @brief Renders the Project Settings modal dialog (opened from the menu bar). */
    void DrawSettingsDialog();
    /** @brief Renders the status bar at the bottom of the viewport. */
    void DrawStatusBar();
    /** @brief Applies the current ProjectSettings to the live engine subsystems. */
    void ApplyProjectSettings();
    /** @brief Reads ProjectInfo.lua from the project root and restores editor state. */
    void LoadProjectInfo();
    /** @brief Loads saved layout .ini files from the project directory. */
    void LoadSavedLayouts();

    Editor(RE::EngineContents engineContents, Project project);
    public:
    /** @brief Writes ProjectInfo.json at the project root with the current editor state. */
    void SaveProjectInfo();

    ~Editor();

    /**
     * @brief Factory method — thin wrapper returning a prvalue for guaranteed copy elision.
     * @param engineContents  Pre-initialised engine bundle (window, GL, etc.).
     * @param project         The project selected on the project-selection screen.
     * @return The new Editor
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

    /** @brief Forwards an editor action event to the active tutorial checklist. */
    void FireTutorialEvent(const std::string& _event) { m_tutorialPanel.NotifyEvent(_event); }

    /** @brief Returns the path to the currently loaded scene. */
    const std::string& GetCurrentScenePath() const { return m_currentScenePath; }
    /** @brief Sets the current scene path. */
    void SetCurrentScenePath(const std::string& _path) { m_currentScenePath = _path; }

    /** @brief Opens the Project Settings modal dialog. */
    void ShowProjectSettingsDialog() { m_showSettingsDialog = true; }
    /** @brief Opens the Theme Settings panel. */
    void ShowThemeSettingsPanel() { m_themeSettingsPanel.SetShown(true); }

    /** @brief Returns true if the given layout slot has saved data. */
    bool HasLayout(int _slot) const { return !m_savedLayouts[_slot].empty(); }

    /** @brief Returns a reference to the ThemeManager. */
    ThemeManager& GetThemeManager() { return m_themeManager; }

    /** @brief Returns a reference to the engine-owned UI renderer. */
    RE::UI::UIRenderer& GetUIRenderer() { return *RE::UI::GetRenderer().lock(); }

    /** @brief Copies the currently selected entities to the clipboard. */
    void CopySelectedEntities();
    /** @brief Pastes entities from the clipboard with an offset. */
    void PasteEntities();
    /** @brief Duplicates the currently selected entities in-place. */
    void DuplicateEntities();

    /** @brief Saves the current ImGui docking layout to the given slot (0-2). */
    void SaveLayout(int _slot);
    /** @brief Loads an ImGui docking layout from the given slot (0-2). */
    void LoadLayout(int _slot);
};

#endif