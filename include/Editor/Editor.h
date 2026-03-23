#ifndef EDITOR_H
#define EDITOR_H

#include "RanakEngine/RanakEngine.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_opengl3.h"

#include <memory>
#include <vector>
#include <string>

class EntityPanel;
class CategoryPanel;
class RulesPanel;
class CameraPanel;

class Editor : public std::enable_shared_from_this<Editor>
{
    private:
    std::shared_ptr<RE::IO::Window> m_window;
    std::shared_ptr<RE::Core::Scene> m_scene;
    std::shared_ptr<RE::Core::Camera> m_camera;

    RE::EngineContents m_engineContents;

    std::shared_ptr<RE::Asset::Shader> m_gridShader;

    // UI Panels
    std::unique_ptr<EntityPanel> m_entityPanel;
    std::unique_ptr<CategoryPanel> m_categoryPanel;
    std::unique_ptr<RulesPanel> m_rulesPanel;
    std::unique_ptr<CameraPanel> m_cameraPanel;

    RE::IO::MouseInfo m_mouseInfo;
    std::shared_ptr<ImFont> m_font;

    int m_selectedEntityId;
    
    bool m_isEditorRunning;
    bool m_isGameRunning;
    
    bool m_showContext;

    void InitImGui();
    void CleanupImGui();
    void DrawMenuBar();
    void DrawDockspace();
    void DrawEditorUI();
    void HandleInput();

    public:
    Editor();
    ~Editor();

    // Factory method to create Editor as shared_ptr
    static std::shared_ptr<Editor> Create();

    void Run();
    void Update(float _deltaTime);
    void Draw();
    
    // Accessors
    std::shared_ptr<RE::Core::Scene> GetScene() { return m_scene; }
    RE::EngineContents& GetEngineContents() { return m_engineContents; }
    int GetSelectedEntityId() const { return m_selectedEntityId; }
    void SetSelectedEntityId(int _id) { m_selectedEntityId = _id; }
    std::weak_ptr<RE::Core::Camera> GetCamera() { return m_camera; }
};

#endif