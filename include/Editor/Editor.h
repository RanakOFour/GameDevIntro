#ifndef EDITOR_H
#define EDITOR_H

#include "RanakEngine/RanakEngine.h"

#include "Editor/TutorialPanel.h"

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
class Editor : public std::enable_shared_from_this<Editor>
{
private:
    std::shared_ptr<SceneEditTab> m_sceneEdit; ///< The 3D scene-editing tab.
    std::shared_ptr<TextEditTab>  m_textEdit;  ///< The Lua source-file editing tab.
    ImFont* m_font;                            ///< Custom font loaded for the editor UI.

    RE::EngineContents m_engineContents; ///< Aggregated engine sub-systems.

    bool m_showTextEdit; ///< True when the TextEditTab is active.

    bool m_showLoadDialog = false;   ///< True when the "Load Category" file dialog is open.
    bool m_showSaveDialog = false;   ///< True when the "Create Category" dialog is open.

    TutorialPanel m_tutorialPanel; ///< In-editor guided tutorial overlay (universal across tabs).

    /** @brief Initialises Dear ImGui (context, style, SDL3/OpenGL backends). */
    void InitImGui();
    /** @brief Tears down Dear ImGui and releases its resources. */
    void CleanupImGui();
    /** @brief Polls SDL events and forwards them to ImGui and the scene camera. */
    void HandleInput();
    /** @brief Renders the application menu bar (universal across tabs). */
    void DrawMenuBar();

    Editor();
    public:
    ~Editor();

    /**
     * @brief Factory method — creates and fully initialises an Editor as a shared_ptr.
     * @return Shared pointer to the new Editor.
     */
    static std::shared_ptr<Editor> Create();

    /** @brief Enters the main loop; returns when the user closes the editor. */
    void Run();
    /** @brief Renders one complete frame (clear, ImGui, swap). */
    void Draw();

    /** @brief Returns a reference to the engine sub-system bundle. */
    RE::EngineContents& GetEngineContents() { return m_engineContents; }
    /** @brief Returns a weak pointer to the SceneEditTab. */
    std::weak_ptr<SceneEditTab> GetSceneEdit();
    /** @brief Returns a reference to the tutorial panel. */
    TutorialPanel& GetTutorialPanel() { return m_tutorialPanel; }
};

#endif