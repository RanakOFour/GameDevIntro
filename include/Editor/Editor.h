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

class Editor : public std::enable_shared_from_this<Editor>
{
    private:
    std::shared_ptr<RE::IO::Window> m_window;
    std::shared_ptr<RE::Core::Scene> m_scene;
    std::shared_ptr<RE::Core::Camera> m_camera;

    RE::EngineContents m_engineContents;

    std::shared_ptr<RE::Asset::Shader> m_gridShader;
    GLuint m_dummyGridVAO = 0;

    // UI Panels
    EntityPanel m_entityPanel;
    CategoryPanel m_categoryPanel;
    RulesPanel m_rulesPanel;
    CameraPanel m_cameraPanel;
    PropertiesPanel m_propertiesPanel;

    RE::IO::MouseInfo m_mouseInfo;
    ImFont* m_font;

    int m_selectedEntityId = -1;
    
    bool m_isEditorRunning = true;
    bool m_isGameRunning = false;
    
    bool m_showContext = false;

    void InitImGui();
    void CleanupImGui();
    void DrawMenuBar();
    void DrawDockspace();
    void DrawEditorUI();
    void HandleInput();

    Editor();
    public:
    ~Editor();

    // Factory method to create Editor as shared_ptr
    static std::shared_ptr<Editor> Create();

    void Run();
    void Update(float _deltaTime);
    void Draw();
    
    
    std::shared_ptr<RE::Core::Scene> GetScene() { return m_scene; }
    RE::EngineContents& GetEngineContents() { return m_engineContents; }
    int GetSelectedEntityId() const { return m_selectedEntityId; }
    void SetSelectedEntityId(int _id);
    std::weak_ptr<RE::Core::Camera> GetCamera() { return m_camera; }
};

#endif