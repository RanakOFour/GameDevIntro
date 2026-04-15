#include "Editor/Editor.h"

#include "Editor/SceneEditTab.h"
#include "Editor/TextEditTab.h"
#include "Editor/AutoCompleteTree.h"

#include "Editor/SceneSerializer.h"
#include "Editor/BuiltinCategories.h"
#include "Editor/BuiltinRules.h"
#include "Editor/Gizmo.h"
#include "Editor/SceneSettings.h"
    
#include "RanakEngine/RanakEngine.h"

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
, m_tutorialPanel(*this)
{
    RE::Log::Message("Engine already initialised; Editor taking ownership");

    // Load built-in (read-only) categories from the platform data directory.
    BuiltinCategories::Load(m_engineContents);

    // Initialize ImGui with the window from IO Manager
    InitImGui();

    m_font = ImGui::GetIO().Fonts->AddFontFromFileTTF("./resources/Fonts/MapleMono.ttf");

    BuiltinRules::Load(m_engineContents);

    // Create tabs — pass *this (Editor fully owns both, both outlived by this).
    m_sceneEdit = std::make_shared<SceneEditTab>(*this);
    m_textEdit  = std::make_shared<TextEditTab>(*this);
    m_textEdit->m_acTree.SetTextEdit(m_textEdit);

    // Register editor-level conditions and actions.
    // Raw pointer capture safe: lambdas stored in m_stateRegistry owned by this Editor.
    Editor* l_raw = this;
    m_stateRegistry.RegisterCondition("scene_tab", [l_raw]{ return l_raw->m_state == State::SceneEdit; });
    m_stateRegistry.RegisterCondition("text_tab",  [l_raw]{ return l_raw->m_state == State::TextEdit; });
    m_stateRegistry.RegisterAction("scene_tab", [l_raw]{ l_raw->m_state = State::SceneEdit; });
    m_stateRegistry.RegisterAction("text_tab",  [l_raw]{ l_raw->m_state = State::TextEdit; });

    RE::Log::Message("Editor initialized with UI panels");

    LoadProjectInfo();
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

        if (ImGui::BeginMenu("Edit"))
        {
            std::string l_undoLabel = m_undoManager.CanUndo()
                ? "Undo " + m_undoManager.GetUndoDescription()
                : "Undo";
            std::string l_redoLabel = m_undoManager.CanRedo()
                ? "Redo " + m_undoManager.GetRedoDescription()
                : "Redo";

            if (ImGui::MenuItem(l_undoLabel.c_str(), "Ctrl+Z", false, m_undoManager.CanUndo()))
                m_undoManager.Undo();
            if (ImGui::MenuItem(l_redoLabel.c_str(), "Ctrl+Shift+Z", false, m_undoManager.CanRedo()))
                m_undoManager.Redo();

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

            if (ImGui::MenuItem("Console"))
            {
                m_sceneEdit->m_consolePanel.SetShown(!m_sceneEdit->m_consolePanel.IsShown());
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::MenuItem("Project Settings"))
                m_showSettingsDialog = true;
            if (ImGui::MenuItem("Scene Settings"))
                m_sceneEdit->m_settingsPanel.SetShown(true);
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
                SceneSerializer::LoadFromFile(l_filePathName, m_engineContents,
                                              &m_sceneEdit->GetSceneSettings());

                // Re-add built-in rules after scene load (they are excluded from serialisation)
                BuiltinRules::Load(m_engineContents);
                // Rebuild the editor rule registry to match the newly loaded scene.
                m_sceneEdit->RebuildRegistryFromScene();
                m_sceneEdit->m_scene = m_engineContents.core->GetScene();
                m_sceneEdit->m_entityPanel.RefreshEntityList();

                m_currentScenePath = l_filePathName;
                SaveProjectInfo();
            }
            else
            {
                SceneSerializer::SaveToFile(l_filePathName, m_engineContents,
                                            m_sceneEdit->GetSceneSettings());
                m_currentScenePath = l_filePathName;
                SaveProjectInfo();
            }
        }

        ImGuiFileDialog::Instance()->Close();
        m_showLoadDialog = false;
        m_showSaveDialog = false;
    }
}

void Editor::ApplyProjectSettings()
{
    const ProjectSettings& s = m_project.GetSettings();

    // Camera: restore initial state.
    if (auto l_camera = m_engineContents.core->GetCamera().lock())
    {
        l_camera->SetPosition(Vector3(s.cameraX, s.cameraY, s.cameraZ));
        l_camera->SetCameraWidth(s.cameraWidth);
        if (s.cameraPerspective) l_camera->SetPerspective();
        else                     l_camera->SetOrthographic();
    }
}

void Editor::DrawSettingsDialog()
{
    if (!m_showSettingsDialog)
        return;

    ImGui::OpenPopup("Project Settings");
    m_showSettingsDialog = false; // consumed — popup stays open via BeginPopupModal

    ImGui::SetNextWindowSize(ImVec2(400, 0), ImGuiCond_Always);
}

// Drawn every frame so the modal persists while open.
static void DrawSettingsDialogContent(ProjectSettings& s, bool& outApply)
{
    outApply = false;

    if (!ImGui::BeginPopupModal("Project Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        return;

    ImGui::SeparatorText("Camera (initial)");
    ImGui::DragFloat("Position X", &s.cameraX, 0.1f, -10000.0f, 10000.0f, "%.2f");
    ImGui::DragFloat("Position Y", &s.cameraY, 0.1f, -10000.0f, 10000.0f, "%.2f");
    ImGui::DragFloat("Position Z", &s.cameraZ, 0.1f,      0.1f, 10000.0f, "%.2f");
    ImGui::DragFloat("Width (ortho zoom)", &s.cameraWidth, 0.1f, 0.1f, 1000.0f, "%.2f");
    ImGui::Checkbox("Perspective", &s.cameraPerspective);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Apply & Close", ImVec2(160, 0)))
    {
        outApply = true;
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(100, 0)))
        ImGui::CloseCurrentPopup();

    ImGui::EndPopup();
}

void Editor::SaveProjectInfo()
{
    if (!m_project.IsOpen())
        return;

    const ProjectSettings& s = m_project.GetSettings();

    json l_projectInfo;
    l_projectInfo["currentScene"] = m_currentScenePath;

    l_projectInfo["settings"]["camera"]["x"]           = s.cameraX;
    l_projectInfo["settings"]["camera"]["y"]           = s.cameraY;
    l_projectInfo["settings"]["camera"]["z"]           = s.cameraZ;
    l_projectInfo["settings"]["camera"]["width"]       = s.cameraWidth;
    l_projectInfo["settings"]["camera"]["perspective"] = s.cameraPerspective;

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
        SceneSerializer::SaveToFile(l_defaultScene, m_engineContents,
                                    m_sceneEdit->GetSceneSettings());
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

    // --- Restore project settings (safe: defaults are used for missing keys) ---
    if (l_projectInfo.contains("settings"))
    {
        const auto& s = l_projectInfo["settings"];
        ProjectSettings& ps = m_project.GetSettings();

        if (s.contains("camera"))
        {
            ps.cameraX           = s["camera"].value("x",           ps.cameraX);
            ps.cameraY           = s["camera"].value("y",           ps.cameraY);
            ps.cameraZ           = s["camera"].value("z",           ps.cameraZ);
            ps.cameraWidth       = s["camera"].value("width",       ps.cameraWidth);
            ps.cameraPerspective = s["camera"].value("perspective", ps.cameraPerspective);
        }
    }

    ApplyProjectSettings();

    // --- Restore last open scene ---
    if (!l_projectInfo.contains("currentScene") || l_projectInfo["currentScene"].is_null())
    {
        return;
    }

    std::string l_scenePath = l_projectInfo["currentScene"].get<std::string>();

    if (l_scenePath.empty() || !std::filesystem::exists(l_scenePath))
    {
        return;
    }

    SceneSerializer::LoadFromFile(l_scenePath, m_engineContents,
                                  &m_sceneEdit->GetSceneSettings());

    // Re-add built-in rules after scene load (they are excluded from serialisation)
    BuiltinRules::Load(m_engineContents);
    // Rebuild the editor rule registry to match the newly loaded scene.
    m_sceneEdit->RebuildRegistryFromScene();
    m_sceneEdit->m_scene = m_engineContents.core->GetScene();
    m_sceneEdit->m_entityPanel.RefreshEntityList();

    m_currentScenePath = l_scenePath;
    printf("Editor: Restored scene from ProjectInfo: %s\n", l_scenePath.c_str());
}

void Editor::Draw()
{
    // Clear the screen using the scene's configured background colour.
    const SceneSettings& ss = m_sceneEdit->GetSceneSettings();
    glClearColor(ss.clearColorR, ss.clearColorG, ss.clearColorB, ss.clearColorA);
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

    // Project Settings modal — drawn outside menu bar so it renders correctly.
    DrawSettingsDialog();
    bool l_applySettings = false;
    DrawSettingsDialogContent(m_project.GetSettings(), l_applySettings);
    if (l_applySettings)
    {
        ApplyProjectSettings();
        SaveProjectInfo();
    }

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
        // First-frame LMB press - check for entity selection or gizmo interaction
        if (l_mouseInfo.LMBDown && !m_engineContents.io->GetLastFrameMouseInfo().LMBDown)
        {
            Vector3 l_mouseWorldPos = m_sceneEdit->m_camera->ScreenToWorldPoint(l_mouseInfo.position);
            l_mouseWorldPos.z = m_sceneEdit->m_camera->GetPosition().z;

            auto l_scene = m_engineContents.core->GetScene().lock();
            SceneSettings& l_ss = m_sceneEdit->GetSceneSettings();

            // Gizmo hit test (has priority over scene pick)
            bool l_gizmoHit = false;
            if (!m_sceneEdit->m_isGameRunning && m_sceneEdit->m_selectedEntityId >= 0)
            {
                Vector2 l_entityWorldPos = l_scene->GetRegistry()
                    .GetEntityAttributes(m_sceneEdit->m_selectedEntityId)
                    .traverse_raw_get<Vector2>("Transform", "Position");
                Vector2 l_entityWorldScale = l_scene->GetRegistry()
                    .GetEntityAttributes(m_sceneEdit->m_selectedEntityId)
                    .traverse_raw_get<Vector2>("Transform", "Scale");
                
                Vector2 l_entityScreenPos = m_sceneEdit->m_camera->WorldToScreenPoint(l_entityWorldPos);
                Vector2 l_screenEdgeX = m_sceneEdit->m_camera->WorldToScreenPoint(
                    l_entityWorldPos + Vector2(l_entityWorldScale.x, 0));
                Vector2 l_screenEdgeY = m_sceneEdit->m_camera->WorldToScreenPoint(
                    l_entityWorldPos + Vector2(0, l_entityWorldScale.y));
                
                float l_screenH = m_engineContents.io->GetWindow().lock()->GetScreenSize().y;
                ImVec2 l_screenHE(std::abs(l_screenEdgeX.x - l_entityScreenPos.x),
                                  std::abs(l_screenEdgeY.y - l_entityScreenPos.y));
                
                                  ImVec2 l_entityScreenIm(l_entityScreenPos.x, l_screenH - l_entityScreenPos.y);

                ImVec2 l_mouseScreenIm(l_mouseInfo.position.x, l_screenH - l_mouseInfo.position.y);

                Gizmo::Axis l_hitAxis = Gizmo::HitTest(
                    l_ss.gizmoMode, l_entityScreenIm, l_screenHE, l_mouseScreenIm);

                if (l_hitAxis != Gizmo::Axis::None)
                {
                    l_gizmoHit = true;
                    m_sceneEdit->m_isDraggingGizmo  = true;
                    m_sceneEdit->m_activeGizmoAxis  = l_hitAxis;
                    m_sceneEdit->m_isDraggingEntity = true;
                    m_sceneEdit->m_dragStartWorldPos  = Vector2(l_mouseWorldPos.x, l_mouseWorldPos.y);
                    m_sceneEdit->m_dragStartEntityPos = l_entityWorldPos;
                    m_sceneEdit->m_dragStartEntityScale = l_scene->GetRegistry()
                        .GetEntityAttributes(m_sceneEdit->m_selectedEntityId)
                        .traverse_raw_get<Vector2>("Transform", "Scale");
                    m_sceneEdit->m_dragStartEntityRot = l_scene->GetRegistry()
                        .GetEntityAttributes(m_sceneEdit->m_selectedEntityId)
                        .traverse_raw_get<float>("Transform", "Rotation");
                }
            }

            // Scene raycast (only if gizmo was not hit)
            if (!l_gizmoHit)
            {
                RE::Core::Ray l_ray{
                    l_mouseWorldPos,
                    Vector3(0.0f, 0.0f, -1.0f)
                };
                RE::Core::RaycastHit l_hitInfo;
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
                    m_sceneEdit->SelectEntity(-1);
                    m_sceneEdit->m_propertiesPanel.SetShown(false);
                }

                RE::Log::Message("Clicked entity: " + std::to_string(l_hitEntity));
            }
        }

        // --- LMB held: drag entity / gizmo ---
        if (l_mouseInfo.LMBDown && m_sceneEdit->m_isDraggingEntity && m_sceneEdit->m_selectedEntityId >= 0)
        {
            Vector3 l_currentWorld = m_sceneEdit->m_camera->ScreenToWorldPoint(l_mouseInfo.position);
            Vector2 l_delta(
                l_currentWorld.x - m_sceneEdit->m_dragStartWorldPos.x,
                l_currentWorld.y - m_sceneEdit->m_dragStartWorldPos.y
            );

            auto l_scene = m_engineContents.core->GetScene().lock();
            sol::table l_transform = l_scene->GetRegistry()
                .GetEntityAttributes(m_sceneEdit->m_selectedEntityId)
                .raw_get<sol::table>("Transform");

            SceneSettings& l_ss = m_sceneEdit->GetSceneSettings();

            if (m_sceneEdit->m_isDraggingGizmo)
            {
                Gizmo::Axis l_axis = m_sceneEdit->m_activeGizmoAxis;

                if (l_axis == Gizmo::Axis::Rotate)
                {
                    // Rotation: angle from entity centre to mouse
                    Vector2 l_entityPos = m_sceneEdit->m_dragStartEntityPos;

                    float l_angle = std::atan2(
                        l_currentWorld.y - l_entityPos.y,
                        l_currentWorld.x - l_entityPos.x);
                    
                    float l_startAngle = std::atan2(
                        m_sceneEdit->m_dragStartWorldPos.y - l_entityPos.y,
                        m_sceneEdit->m_dragStartWorldPos.x - l_entityPos.x);
                    
                    float l_deltaAngle = l_angle - l_startAngle;
                    
                    // Wrap delta to [-PI, PI]
                    l_deltaAngle = std::atan2(std::sin(l_deltaAngle), std::cos(l_deltaAngle));
                    
                    // Convert to degrees and add to start rotation
                    float l_deltaDeg = l_deltaAngle * (180.0f / RE::Math::PI());
                    float l_newRot = m_sceneEdit->m_dragStartEntityRot + l_deltaDeg;
                    
                    // Wrap to (-180, 180]
                    if(l_newRot > 180.0f) l_newRot -= 360.0f;
                    else if(l_newRot <= -180.0f) l_newRot += 360.0f;

                    l_transform.raw_set("Rotation", l_newRot);
                }
                else if (l_axis == Gizmo::Axis::ScaleX || l_axis == Gizmo::Axis::ScaleY || l_axis == Gizmo::Axis::ScaleXY)
                {
                    // Scale: delta mapped to scale change
                    Vector2 l_constrained = Gizmo::ConstrainDelta(l_axis, l_delta);
                    Vector2 l_newScale = m_sceneEdit->m_dragStartEntityScale + l_constrained;
                    
                    // Clamp to prevent negative/zero scale
                    if (l_newScale.x < 0.01f) l_newScale.x = 0.01f;
                    if (l_newScale.y < 0.01f) l_newScale.y = 0.01f;
                    l_transform.raw_set("Scale", l_newScale);
                }
                else
                {
                    // Translate with axis constraint
                    Vector2 l_constrained = Gizmo::ConstrainDelta(l_axis, l_delta);
                    Vector2 l_newPos = m_sceneEdit->m_dragStartEntityPos + l_constrained;
                    if (l_ss.snapEnabled)
                        l_newPos = Gizmo::Snap(l_newPos, l_ss.snapGridSize);
                    l_transform.raw_set("Position", l_newPos);
                }
            }
            else
            {
                // Free drag (no gizmo)
                Vector2 l_newPos = m_sceneEdit->m_dragStartEntityPos + l_delta;
                if (l_ss.snapEnabled)
                    l_newPos = Gizmo::Snap(l_newPos, l_ss.snapGridSize);
                l_transform.raw_set("Position", l_newPos);
            }
        }

        // --- LMB released: end drag, push undo ---
        if (!l_mouseInfo.LMBDown && m_sceneEdit->m_isDraggingEntity)
        {
            m_sceneEdit->m_isDraggingEntity = false;
            m_sceneEdit->m_isDraggingGizmo  = false;
            m_sceneEdit->m_activeGizmoAxis  = Gizmo::Axis::None;

            if (m_sceneEdit->m_selectedEntityId >= 0)
            {
                auto l_scene = m_engineContents.core->GetScene().lock();
                sol::table l_transform = l_scene->GetRegistry()
                    .GetEntityAttributes(m_sceneEdit->m_selectedEntityId)
                    .raw_get<sol::table>("Transform");

                Vector2 l_finalPos   = l_transform.raw_get<Vector2>("Position");
                Vector2 l_finalScale = l_transform.raw_get<Vector2>("Scale");
                float   l_finalRot   = l_transform.raw_get<float>("Rotation");

                Vector2 l_oldPos   = m_sceneEdit->m_dragStartEntityPos;
                Vector2 l_oldScale = m_sceneEdit->m_dragStartEntityScale;
                float   l_oldRot   = m_sceneEdit->m_dragStartEntityRot;

                // Only push undo if something changed
                bool l_posChanged   = (l_finalPos.x != l_oldPos.x || l_finalPos.y != l_oldPos.y);
                bool l_scaleChanged = (l_finalScale.x != l_oldScale.x || l_finalScale.y != l_oldScale.y);
                bool l_rotChanged   = (l_finalRot != l_oldRot);

                if (l_posChanged || l_scaleChanged || l_rotChanged)
                {
                    int l_entityId = m_sceneEdit->m_selectedEntityId;
                    Editor* l_self = this;

                    m_undoManager.PushCommand(
                        std::make_unique<LambdaCommand>(
                            "Transform Entity",
                            [l_self, l_entityId, l_finalPos, l_finalScale, l_finalRot]() {
                                auto l_sc = l_self->GetEngineContents().core->GetScene().lock();
                                sol::table l_tf = l_sc->GetRegistry()
                                    .GetEntityAttributes(l_entityId)
                                    .raw_get<sol::table>("Transform");
                                l_tf.raw_set("Position", l_finalPos);
                                l_tf.raw_set("Scale", l_finalScale);
                                l_tf.raw_set("Rotation", l_finalRot);
                            },
                            [l_self, l_entityId, l_oldPos, l_oldScale, l_oldRot]() {
                                auto l_sc = l_self->GetEngineContents().core->GetScene().lock();
                                sol::table l_tf = l_sc->GetRegistry()
                                    .GetEntityAttributes(l_entityId)
                                    .raw_get<sol::table>("Transform");
                                l_tf.raw_set("Position", l_oldPos);
                                l_tf.raw_set("Scale", l_oldScale);
                                l_tf.raw_set("Rotation", l_oldRot);
                            }
                        )
                    );
                }
            }
        }

        m_sceneEdit->m_camera->SetCameraWidth(m_sceneEdit->m_camera->GetCameraWidth() + l_mouseInfo.deltaScroll);
    }

    if (!ImGui::GetIO().WantTextInput)
    {
        // Gizmo mode shortcuts (W/E/R — Unity convention)
        SceneSettings& l_ss = m_sceneEdit->GetSceneSettings();
        if (m_engineContents.io->GetKeyDownThisFrame('w'))
            l_ss.gizmoMode = GizmoMode::Translate;
        if (m_engineContents.io->GetKeyDownThisFrame('e'))
            l_ss.gizmoMode = GizmoMode::Rotate;
        if (m_engineContents.io->GetKeyDownThisFrame('r'))
            l_ss.gizmoMode = GizmoMode::Scale;

        // Snap toggle (Ctrl+G)
        if (ImGui::GetIO().KeyCtrl && m_engineContents.io->GetKeyDownThisFrame('g'))
            l_ss.snapEnabled = !l_ss.snapEnabled;

        if (m_engineContents.io->GetKeyDownThisFrame('c'))
        {
            m_sceneEdit->m_categoryPanel.SetShown(!m_sceneEdit->m_categoryPanel.IsShown());
        }

        if (m_engineContents.io->GetKeyDownThisFrame('`'))
        {
            m_sceneEdit->m_consolePanel.SetShown(!m_sceneEdit->m_consolePanel.IsShown());
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

        // Undo / Redo shortcuts
        if (ImGui::GetIO().KeyCtrl && ImGui::GetIO().KeyShift && m_engineContents.io->GetKeyDownThisFrame('z'))
        {
            m_undoManager.Redo();
        }
        else if (ImGui::GetIO().KeyCtrl && m_engineContents.io->GetKeyDownThisFrame('z'))
        {
            m_undoManager.Undo();
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

SceneEditTab& Editor::GetSceneEdit()
{
    return *m_sceneEdit;
}