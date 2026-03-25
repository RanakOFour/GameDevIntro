#ifndef EDITOR_H
#define EDITOR_H

#include "RanakEngine/RanakEngine.h"

#include "Editor/EntityPanel.h"
#include "Editor/CategoryPanel.h"
#include "Editor/RulesPanel.h"
#include "Editor/PropertiesPanel.h"
#include "Editor/CameraPanel.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_opengl3.h"

#include <memory>
#include <vector>
#include <string>

class SceneEditTab;
class TextEditTab;
class Editor : public std::enable_shared_from_this<Editor>
{
    private:
    std::shared_ptr<SceneEditTab> m_sceneEdit;
    std::shared_ptr<TextEditTab> m_textEdit;
    ImFont* m_font;
    
    RE::EngineContents m_engineContents;
    
    int m_tabIndex = 0;

    void InitImGui();
    void CleanupImGui();
    void HandleInput();

    Editor();
    public:
    ~Editor();

    // Factory method to create Editor as shared_ptr
    static std::shared_ptr<Editor> Create();

    void Run();
    void Draw();
    
    RE::EngineContents& GetEngineContents() { return m_engineContents; }
    std::weak_ptr<SceneEditTab> GetSceneEdit();
};

#endif