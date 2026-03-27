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

class SceneEditTab
{
    friend Editor;
private:
	std::weak_ptr<Editor> m_editor;
	std::shared_ptr<RE::IO::Window> m_window;
	std::shared_ptr<RE::Core::Scene> m_scene;
	std::shared_ptr<RE::Core::Camera> m_camera;

    std::shared_ptr<RE::Asset::Shader> m_gridShader;
    GLuint m_dummyGridVAO = 0;

    // UI Panels
    EntityPanel m_entityPanel;
    CategoryPanel m_categoryPanel;
    RulesPanel m_rulesPanel;
    CameraPanel m_cameraPanel;
    PropertiesPanel m_propertiesPanel;
    TutorialPanel m_tutorialPanel;

    int m_selectedEntityId = -1;

    bool m_isEditorRunning = true;
    bool m_isGameRunning = false;

    bool m_showContext = false;

    void DrawEditorUI();
    void DrawContextMenu();
    void HandleInput();

public:
    SceneEditTab(std::weak_ptr<Editor> _editor);
    ~SceneEditTab();

    void Draw();
    void DrawMenuBar();

    void Run();

    void SelectEntity(int _id);
    int GetSelectedEntity();
};

#endif