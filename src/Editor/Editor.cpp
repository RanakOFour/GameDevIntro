#include "Editor/Editor.h"

#include "Editor/SceneEditTab.h"
#include "Editor/TextEditTab.h"
#include "Editor/AutoCompleteTree.h"

#include "Editor/SceneSerializer.h"
#include "Editor/BuiltinCategories.h"

#include "RanakEngine/IO.h"
#include "RanakEngine/Core.h"

#include "imguiFileDialog/ImGuiFileDialog.h"

#include "json/json.hpp"
using json = nlohmann::json;

#include "SDL3/SDL.h"
#include <GL/gl.h>
#include <fstream>
#include <filesystem>

Editor::Editor(RE::EngineContents engineContents, Project project)
: m_state(State::SceneEdit)
, m_showLoadDialog(false)
, m_showSaveDialog(false)
, m_engineContents(std::move(engineContents))
, m_project(std::move(project))
{
    RE::Log::Message("Engine already initialised; Editor taking ownership");

    // Load built-in (read-only) categories from the platform data directory.
    BuiltinCategories::Load(m_engineContents);

    // Initialize ImGui with the window from IO Manager
    InitImGui();

    m_font = ImGui::GetIO().Fonts->AddFontFromFileTTF("./resources/Fonts/MapleMono.ttf");

    auto l_renderRuleFile = m_engineContents.resources->Load<RE::Asset::LuaFile>("./resources/Rules/EditorRender.lua");
    RE::Core::Rule l_renderRule = m_engineContents.core->GetLuaContext()->CreateRule(l_renderRuleFile);

    m_engineContents.core->GetScene().lock()->AddRule(l_renderRule);

    RE::Log::Message("Editor constructed");
}

std::shared_ptr<Editor> Editor::Create(RE::EngineContents engineContents, Project project)
{
    std::shared_ptr<Editor> l_editor;
    l_editor.reset(new Editor(std::move(engineContents), std::move(project)));

    std::shared_ptr<Editor> l_editorFromThis = l_editor->shared_from_this();
    
    l_editor->m_tutorialPanel = TutorialPanel(l_editorFromThis);
    l_editor->m_sceneEdit = std::make_shared<SceneEditTab>(l_editorFromThis);
    l_editor->m_textEdit = std::make_shared<TextEditTab>(l_editorFromThis);
    l_editor->m_textEdit->m_acTree.SetTextEdit(l_editor->m_textEdit);

    // Register editor-level conditions and actions.
    // Raw pointer capture is safe: lambdas live inside m_stateRegistry which is
    // owned by the same Editor object.
    Editor* l_raw = l_editor.get();
    l_editor->m_stateRegistry.RegisterCondition("scene_tab", [l_raw]{ return l_raw->m_state == State::SceneEdit; });
    l_editor->m_stateRegistry.RegisterCondition("text_tab",  [l_raw]{ return l_raw->m_state == State::TextEdit; });
    l_editor->m_stateRegistry.RegisterAction("scene_tab", [l_raw]{ l_raw->m_state = State::SceneEdit; });
    l_editor->m_stateRegistry.RegisterAction("text_tab",  [l_raw]{ l_raw->m_state = State::TextEdit; });

    RE::Log::Message("Editor initialized with UI panels");

    l_editor->LoadProjectInfo();

    return l_editor;
}

Editor::~Editor()
{
    SaveProjectInfo();

    m_sceneEdit.reset();
    m_textEdit.reset();

    // Clean up ImGui
    CleanupImGui();

    // Shut down engine
    RE::Shutdown(m_engineContents);

    // Log is shutdown, so now we print
    printf("Editor shutdown complete\n");
}

void Editor::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    io.ConfigDragClickToInputText = 0.0f;

    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowMenuButtonPosition = ImGuiDir_Right;

    auto l_window = m_engineContents.io->GetWindow().lock();
    
    ImGui_ImplSDL3_InitForOpenGL(l_window->GetSDLWindow(), l_window->GetGLContext());
    ImGui_ImplOpenGL3_Init("#version 430");

    RE::Log::Message("ImGui initialized");
}

void Editor::CleanupImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void Editor::Run()
{
    const float l_targetFrameTime = 1.0f / 60.0f;
    const Uint64 l_perfFreq = SDL_GetPerformanceFrequency();
    Uint64 l_frameStart = SDL_GetPerformanceCounter();

    while (!m_engineContents.io->GetQuitSignal())
    {
        HandleInput();
        //Update(l_targetFrameTime);
        Draw();

        // Measure how long this iteration took and sleep for the remainder of
        // the 60 Hz budget. SDL_Delay granularity is ~1 ms so we keep spinning
        // for the last millisecond to hit the target precisely.
        float l_elapsed = (SDL_GetPerformanceCounter() - l_frameStart) / static_cast<float>(l_perfFreq);
        float l_remaining = l_targetFrameTime - l_elapsed;
        if (l_remaining > 0.001f)
        {
            SDL_Delay((Uint32)((l_remaining - 0.001f) * 1000.0f));
        }
        while ((SDL_GetPerformanceCounter() - l_frameStart) / static_cast<float>(l_perfFreq) < l_targetFrameTime)
        {
            // Silly wait for sub-millisecond accuracy
        }

        l_frameStart = SDL_GetPerformanceCounter();
    }

    printf("Editor no longer running\n");
}

void Editor::DrawMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene", "Ctrl+N"))
            {
                
            }
            if (ImGui::MenuItem("Load Scene", "Ctrl+O"))
            {
                IGFD::FileDialogConfig config;
                config.path = m_project.IsOpen() ? m_project.GetScenesDir() : ".";
                ImGuiFileDialog::Instance()->OpenDialog("SceneFileDlgKey", "Choose File", ".lua", config);
                m_showLoadDialog = true;
                m_showSaveDialog = false;
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
            {
                IGFD::FileDialogConfig config;
                config.path = m_project.IsOpen() ? m_project.GetScenesDir() : ".";
                ImGuiFileDialog::Instance()->OpenDialog("SceneFileDlgKey", "Choose File", ".lua", config);
                m_showLoadDialog = false;
                m_showSaveDialog = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Ctrl+Q"))
            {
                SDL_Event l_quitEvent;
                l_quitEvent.type = SDL_EVENT_QUIT;
                SDL_PushEvent(&l_quitEvent);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Show Grid");
            ImGui::MenuItem("Show Gizmos");

            if (ImGui::MenuItem("Camera Settings"))
            {
                m_sceneEdit->m_cameraPanel.SetShown(true);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tutorials"))
        {
            if (ImGui::MenuItem("Getting Started"))
                m_tutorialPanel.LoadTutorial("./resources/Tutorials/GettingStarted.lua");
            if (ImGui::MenuItem("Creating Categories"))
                m_tutorialPanel.LoadTutorial("./resources/Tutorials/Categories.lua");
            if (ImGui::MenuItem("Writing Rules"))
                m_tutorialPanel.LoadTutorial("./resources/Tutorials/Rules.lua");
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (ImGuiFileDialog::Instance()->Display("SceneFileDlgKey"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string l_filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string l_filePathDir = ImGuiFileDialog::Instance()->GetCurrentPath();
            if (m_showLoadDialog)
            {
                SceneSerializer::LoadFromFile(l_filePathName, m_engineContents);

                // Re-add EditorRender rule, since it is ignored during scene serialisation
                auto l_renderRuleFile = m_engineContents.resources->Load<RE::Asset::LuaFile>("./resources/Rules/EditorRender.lua");
                RE::Core::Rule l_renderRule = m_engineContents.core->GetLuaContext()->CreateRule(l_renderRuleFile);

                m_engineContents.core->GetScene().lock()->AddRule(l_renderRule);
                m_sceneEdit->m_scene = m_engineContents.core->GetScene();
                m_sceneEdit->m_entityPanel.RefreshEntityList();

                m_currentScenePath = l_filePathName;
                SaveProjectInfo();
            }
            else
            {
                SceneSerializer::SaveToFile(l_filePathName, m_engineContents);
                m_currentScenePath = l_filePathName;
                SaveProjectInfo();
            }
        }

        ImGuiFileDialog::Instance()->Close();
        m_showLoadDialog = false;
        m_showSaveDialog = false;
    }
}

void Editor::SaveProjectInfo()
{
    if (!m_project.IsOpen())
        return;

    json l_projectInfo;
    l_projectInfo["currentScene"] = m_currentScenePath;

    std::ofstream l_file(m_project.GetProjectInfoPath());
    if (!l_file.is_open())
    {
        printf("Editor: Failed to write ProjectInfo.json\n");
        return;
    }

    l_file << l_projectInfo.dump(4);

    l_file.close();
}

void Editor::LoadProjectInfo()
{
    if (!m_project.IsOpen())
    {
        return;
    }

    std::string l_infoPath = m_project.GetProjectInfoPath();
    std::ifstream l_file(l_infoPath);
    if (!l_file.is_open())
    {
        // Fresh project — create and select a default empty scene.
        std::string l_defaultScene = (std::filesystem::path(m_project.GetScenesDir()) / "Default.lua").string();
        SceneSerializer::SaveToFile(l_defaultScene, m_engineContents);
        m_currentScenePath = l_defaultScene;
        printf("Editor: Created default scene: %s\n", l_defaultScene.c_str());
        SaveProjectInfo();
        return;
    }

    json l_projectInfo;
    try 
    {
        l_file >> l_projectInfo;
    }
    catch (json::parse_error& e)
    {
        printf("Editor: Failed to parse ProjectInfo.json: %s\n", e.what());
        return;
    }

    if (!l_projectInfo.contains("currentScene") || l_projectInfo["currentScene"].is_null())
    {
        return;
    }

    std::string l_scenePath = l_projectInfo["currentScene"].get<std::string>();

    if (l_scenePath.empty() || !std::filesystem::exists(l_scenePath))
    {
        return;
    }

    SceneSerializer::LoadFromFile(l_scenePath, m_engineContents);

    // Re-add EditorRender rule, since it is ignored during scene serialisation
    auto l_renderRuleFile = m_engineContents.resources->Load<RE::Asset::LuaFile>("./resources/Rules/EditorRender.lua");
    RE::Core::Rule l_renderRule = m_engineContents.core->GetLuaContext()->CreateRule(l_renderRuleFile);
    m_engineContents.core->GetScene().lock()->AddRule(l_renderRule);
    m_sceneEdit->m_scene = m_engineContents.core->GetScene();
    m_sceneEdit->m_entityPanel.RefreshEntityList();

    m_currentScenePath = l_scenePath;
    printf("Editor: Restored scene from ProjectInfo: %s\n", l_scenePath.c_str());
}

void Editor::Draw()
{
    // Clear the screen
    glClearColor(0.2f, 0.2f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    ImGui::PushFont(m_font, 17.5f);

    // Clear tutorial highlight regions registered last frame
    m_tutorialPanel.ClearRegions();

    // Menu bar and tutorial panel are universal across all tabs
    DrawMenuBar();

    if (m_state == State::TextEdit)
    {
        ImGui::BeginDisabled();
    }

    m_sceneEdit->Draw();

    if (m_state == State::TextEdit)
    {
        ImGui::EndDisabled();
        m_textEdit->Draw();
    }

    // Tutorial panel drawn last so it appears above all tab content.
    m_tutorialPanel.DrawAsWindow();

    // Force the tutorial window to the front of the display stack so it is not
    // obscured by the fullscreen TextEdit window.
    if (m_tutorialPanel.IsActive())
    {
        ImGuiWindow* l_tutWin = ImGui::FindWindowByName(m_tutorialPanel.GetTitle().c_str());
        if (l_tutWin)
            ImGui::BringWindowToDisplayFront(l_tutWin);
    }

    // Rendering
    ImGui::PopFont();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    m_engineContents.io->GetWindow().lock()->Swap();
}

void Editor::HandleInput()
{
    std::vector<SDL_Event> l_polledEvents = m_engineContents.io->UpdateInputs();

    // Handle SDL_Events for imgui
    bool l_windowResized = false;
    for (const SDL_Event& l_event : l_polledEvents)
    {
        ImGui_ImplSDL3_ProcessEvent(&l_event);
    }

    RE::IO::MouseInfo l_mouseInfo = m_engineContents.io->GetMouseInfo();

    if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
    {
        if (l_mouseInfo.LMBDown && !m_engineContents.io->GetLastFrameMouseInfo().LMBDown)
        {
            Vector3 l_mouseWorldPos = m_sceneEdit->m_camera->ScreenToWorldPoint(l_mouseInfo.position);
            l_mouseWorldPos.z = m_sceneEdit->m_camera->GetPosition().z;
            //Raycast into screen to check for object
            RE::Core::Ray l_ray{
                l_mouseWorldPos,
                Vector3(0.0f, 0.0f, -1.0f)
            };

            RE::Core::RaycastHit l_hitInfo;

            auto l_scene = m_engineContents.core->GetScene().lock();

            int l_hitEntity = l_scene->Raycast(l_ray, l_hitInfo);
            
            if (l_hitEntity > -1)
            {
                m_sceneEdit->SelectEntity(l_hitEntity);
                ImVec2 l_panelSize = m_sceneEdit->m_propertiesPanel.GetSize();

                RE::Core::EntityRegistry& l_registry = l_scene->GetRegistry();

                Vector2 l_entityWorldPos = l_registry.GetEntityAttributes(l_hitEntity).traverse_raw_get<Vector2>("Transform", "Position");

                Vector2 l_entityScreenPos = m_sceneEdit->m_camera->WorldToScreenPoint(l_entityWorldPos);

                m_sceneEdit->m_propertiesPanel.SetPosition(ImVec2(l_entityScreenPos.x + l_panelSize.x * 0.25f, l_entityScreenPos.y - l_panelSize.y * 0.25f));
            }
            else
            {
                m_sceneEdit->m_propertiesPanel.SetShown(false);
            }

            RE::Log::Message("Clicked entity: " + std::to_string(l_hitEntity));
        }

        m_sceneEdit->m_camera->SetCameraWidth(m_sceneEdit->m_camera->GetCameraWidth() + l_mouseInfo.deltaScroll);
    }

    if (!ImGui::GetIO().WantTextInput)
    {
        if (m_engineContents.io->GetKeyDownThisFrame('c'))
        {
            m_sceneEdit->m_categoryPanel.SetShown(!m_sceneEdit->m_categoryPanel.IsShown());
        }

        if (m_engineContents.io->GetKeyDownThisFrame('e'))
        {
            m_sceneEdit->m_entityPanel.SetShown(!m_sceneEdit->m_entityPanel.IsShown());
        }

        if (m_engineContents.io->GetKeyDownThisFrame('r'))
        {
            m_sceneEdit->m_rulesPanel.SetShown(!m_sceneEdit->m_rulesPanel.IsShown());
        }

        // ESC input to close all panels and context menus
        if (m_engineContents.io->GetKeyDownThisFrame((char)27))
        {
            m_sceneEdit->m_categoryPanel.SetShown(false);
            m_sceneEdit->m_entityPanel.SetShown(false);
            m_sceneEdit->m_rulesPanel.SetShown(false);
            m_sceneEdit->m_propertiesPanel.SetShown(false);
            m_sceneEdit->m_showContext = false;
        }

        // Tab input to switch between scene and text edit
        if(m_engineContents.io->GetKeyDownThisFrame('\t'))
        {
            if(m_state == State::SceneEdit)
            {
                SetState(State::TextEdit);
            }
            else
            {
                SetState(State::SceneEdit);
            }

            RE::Log::Message("Tab pressed!");
        }
    }
}

std::weak_ptr<SceneEditTab> Editor::GetSceneEdit()
{
    return m_sceneEdit;
}