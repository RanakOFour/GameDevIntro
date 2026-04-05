#include "Editor/Editor.h"

#include "Editor/SceneEditTab.h"
#include "Editor/TextEditTab.h"
#include "Editor/AutoCompleteTree.h"

#include "Editor/SceneSerializer.h"
#include "Editor/BuiltinCategories.h"

#include "RanakEngine/IO.h"
#include "RanakEngine/Core.h"

#include "imguiFileDialog/ImGuiFileDialog.h"

#include "SDL3/SDL.h"
#include <GL/gl.h>

Editor::Editor(RE::EngineContents engineContents, Project project)
: m_showTextEdit(false)
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

    RE::Log::Message("Editor initialized with UI panels");
    
    return l_editor;
}

Editor::~Editor()
{
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
    // Get ImGui context from IO Manager
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Enable mouse drag for window operations
    io.ConfigDragClickToInputText = 0.0f;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Tweak style for better docking
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowMenuButtonPosition = ImGuiDir_Right;

    auto l_window = m_engineContents.io->GetWindow().lock();
    // Setup Platform/Renderer backends
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
    float l_oneSixtieth = 1.0f / 60.0f;
    while (!m_engineContents.io->GetQuitSignal())
    {
        HandleInput();
        //Update(l_oneSixtieth);
        Draw();
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
            }
            else
            {
                SceneSerializer::SaveToFile(l_filePathName, m_engineContents);
            }
        }

        ImGuiFileDialog::Instance()->Close();
        m_showLoadDialog = false;
        m_showSaveDialog = false;
    }
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

    if (m_showTextEdit)
    {
        ImGui::BeginDisabled();
    }

    m_sceneEdit->Draw();

    if (m_showTextEdit)
    {
        ImGui::EndDisabled();
        m_textEdit->Draw();
    }

    // Tutorial panel drawn last so it appears above all tab content
    m_tutorialPanel.DrawAsWindow();

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

        // ESC input
        if (m_engineContents.io->GetKeyDownThisFrame((char)27))
        {
            m_sceneEdit->m_categoryPanel.SetShown(false);
            m_sceneEdit->m_entityPanel.SetShown(false);
            m_sceneEdit->m_rulesPanel.SetShown(false);
            m_sceneEdit->m_propertiesPanel.SetShown(false);
        }

        if(m_engineContents.io->GetKeyDownThisFrame('\t'))
        {
            m_showTextEdit = !m_showTextEdit;
            RE::Log::Message("Tab pressed!");
        }
    }
}

std::weak_ptr<SceneEditTab> Editor::GetSceneEdit()
{
    return m_sceneEdit;
}