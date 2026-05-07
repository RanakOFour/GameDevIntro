#include "Editor/Core/Editor.h"

#include "Editor/Tabs/SceneEditTab.h"
#include "Editor/Tabs/TextEditTab.h"

#include "Editor/Support/AutoCompleteTree.h"

#include "Editor/Scene/SceneSerializer.h"
#include "Editor/Scene/BuiltIn/BuiltinCategories.h"
#include "Editor/Scene/BuiltIn/BuiltinRules.h"
#include "Editor/Scene/BuiltIn/BuiltInTutorials.h"
#include "Editor/Scene/BuiltIn/EditorAssets.h"

#include "Editor/Scene/Gizmo.h"
#include "Editor/Scene/SceneSettings.h"
    
#include "RanakEngine/RanakEngine.h"

#include "imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_opengl3.h"

#include "json/json.hpp"
using json = nlohmann::json;

#include <GL/gl.h>
#include <fstream>
#include <filesystem>
#include <sstream>

Editor::Editor(RE::EngineContents& engineContents, Project& project)
: m_state(State::SceneEdit)
, m_engineContents(engineContents)
, m_project(project)
, m_tutorialPanel(*this)
, m_topBar(*this)
, m_themeSettingsPanel(*this, m_themeManager)
{
    m_editorTable = m_engineContents.core->GetLuaContext()->GetState()->create_named_table("Editor");
    m_editorTable["GetTempPath"] = []() {
                                            return RE::Asset::GetTempDir().string();
                                        };

    // Load default editor texture
    std::filesystem::path l_editorTexPath = RE::Asset::GetTempDir() / "Textures" / "REDefaultTexture.png";
	RE::Asset::CreateIfNotExists(
                                 l_editorTexPath.string(), 
                                 reinterpret_cast<const char*>(EditorAssets::DefaultEditorTexture),
                                 EditorAssets::DefaultEditorTextureSize
                                );

    // Load built-in (read-only) categories from the platform data directory.
    BuiltinCategories::Load(m_engineContents);
    BuiltinRules::Load(m_engineContents);

    // Initialize ImGui with the window from IO Manager
    InitImGui();

    ImFontConfig l_fontConfig;
    l_fontConfig.FontDataOwnedByAtlas = false;
    m_font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*)RE::UI::DefaultFontData(), RE::UI::DefaultFontDataSize(), 16.0f, &l_fontConfig);

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
    LoadSavedLayouts();

    // Apply saved theme (or default) after ImGui context is ready.
    m_themeManager.ApplyPreset(m_themeManager.GetActiveIndex());

    BuiltinTutorials::Load(m_tutorialRegistry);
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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
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

    l_projectInfo["settings"]["camera"]["x"] = s.cameraX;
    l_projectInfo["settings"]["camera"]["y"] = s.cameraY;
    l_projectInfo["settings"]["camera"]["z"] = s.cameraZ;
    l_projectInfo["settings"]["camera"]["width"] = s.cameraWidth;
    l_projectInfo["settings"]["camera"]["perspective"] = s.cameraPerspective;

    // Save user (non-built-in) categories with their source file paths.
    {
        auto l_luaContext = m_engineContents.core->GetLuaContext();
        std::stringstream l_names(l_luaContext->GetCategoryNames());
        std::string l_seg;
        json l_cats = json::array();
        while (std::getline(l_names, l_seg, ';'))
        {
            if (l_seg.empty() || BuiltinCategories::IsBuiltin(l_seg))
                continue;
            auto l_cat = l_luaContext->GetCategory(l_seg).lock();
            if (!l_cat) continue;
            auto l_originFile = l_cat->GetOriginFile().lock();
            if (!l_originFile) continue;
            json l_entry;
            l_entry["name"] = l_seg;
            l_entry["path"] = l_originFile->GetPath();
            l_cats.push_back(l_entry);
        }
        l_projectInfo["categories"] = l_cats;
    }

    // Save user rules from the editor rule registry.
    {
        json l_rules = json::array();
        for (const auto& rec : m_sceneEdit->GetRuleRegistry())
        {
            json l_entry;
            l_entry["name"] = rec.name;
            l_entry["path"] = rec.filePath;
            l_rules.push_back(l_entry);
        }
        l_projectInfo["rules"] = l_rules;
    }

    // Save parent-child hierarchy.
    {
        json l_hierarchy = json::array();
        auto l_scene = m_engineContents.core->GetScene().lock();
        if (l_scene)
        {
            std::vector<int> l_allIds = l_scene->GetRegistry().GetAllRegisteredIds();
            for (int l_id : l_allIds)
            {
                int l_parent = m_sceneEdit->GetEntityParent(l_id);
                if (l_parent >= 0)
                {
                    json l_entry;
                    l_entry["child"]  = l_id;
                    l_entry["parent"] = l_parent;
                    l_hierarchy.push_back(l_entry);
                }
            }
        }
        l_projectInfo["hierarchy"] = l_hierarchy;
    }

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

    // Restore project settings
    if (l_projectInfo.contains("settings"))
    {
        const auto& s = l_projectInfo["settings"];
        ProjectSettings& ps = m_project.GetSettings();

        if (s.contains("camera"))
        {
            ps.cameraX = s["camera"].value("x", ps.cameraX);
            ps.cameraY = s["camera"].value("y", ps.cameraY);
            ps.cameraZ = s["camera"].value("z", ps.cameraZ);
            ps.cameraWidth = s["camera"].value("width", ps.cameraWidth);
            ps.cameraPerspective = s["camera"].value("perspective", ps.cameraPerspective);
        }
    }

    ApplyProjectSettings();

    // Restore scene if present
    if (l_projectInfo.contains("currentScene") && !l_projectInfo["currentScene"].is_null())
    {
        std::string l_scenePath = l_projectInfo["currentScene"].get<std::string>();
        if (!l_scenePath.empty() && std::filesystem::is_regular_file(l_scenePath))
        {
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
    }

    // Restore user categories that aren't already registered
    if (l_projectInfo.contains("categories") && l_projectInfo["categories"].is_array())
    {
        auto l_luaContext = m_engineContents.core->GetLuaContext();
        for (const auto& l_entry : l_projectInfo["categories"])
        {
            std::string l_path = l_entry.value("path", "");
            std::string l_name = l_entry.value("name", "");
            if (l_path.empty() || !std::filesystem::is_regular_file(l_path))
                continue;
            // Skip if already loaded (e.g. referenced by the scene)
            if (l_luaContext->GetCategory(l_name).lock())
                continue;
            auto l_catFile = m_engineContents.resources->Load<RE::Asset::LuaFile>(l_path);
            l_luaContext->CreateCategory(l_catFile);
            printf("Editor: Restored category '%s' from %s\n", l_name.c_str(), l_path.c_str());
        }
    }

    // Restore user rules that aren't already in the scene / registry
    if (l_projectInfo.contains("rules") && l_projectInfo["rules"].is_array())
    {
        auto l_scene      = m_engineContents.core->GetScene().lock();
        auto l_luaContext = m_engineContents.core->GetLuaContext();
        sol::table l_rulesTable = l_scene->GetSceneTable().raw_get<sol::table>("Rules");

        for (const auto& l_entry : l_projectInfo["rules"])
        {
            std::string l_path = l_entry.value("path", "");
            std::string l_name = l_entry.value("name", "");
            if (l_path.empty() || !std::filesystem::is_regular_file(l_path))
                continue;

            // Check if already in the registry (added by RebuildRegistryFromScene)
            bool l_inRegistry = false;
            for (const auto& rec : m_sceneEdit->GetRuleRegistry())
            {
                if (rec.name == l_name) { l_inRegistry = true; break; }
            }

            auto l_existing = l_rulesTable.raw_get<sol::object>(l_name);
            bool l_inScene  = l_existing.valid() && l_existing.get_type() != sol::type::nil;

            if (l_inScene)
            {
                // Present in scene but may be missing from registry if the scene was
                // freshly loaded before the registry was rebuilt — register it.
                if (!l_inRegistry)
                    m_sceneEdit->RegisterRule(l_name, l_path);
                continue;
            }

            // Not in scene: load the file and add the rule.
            auto l_ruleFile = m_engineContents.resources->Load<RE::Asset::LuaFile>(l_path);
            RE::Core::Rule l_rule = l_luaContext->CreateRule(l_ruleFile);
            l_scene->AddRule(l_rule);
            m_sceneEdit->RegisterRule(l_name, l_path);
            printf("Editor: Restored rule '%s' from %s\n", l_name.c_str(), l_path.c_str());
        }
    }

    // Restore parent-child hierarchy.
    if (l_projectInfo.contains("hierarchy") && l_projectInfo["hierarchy"].is_array())
    {
        for (auto& l_entry : l_projectInfo["hierarchy"])
        {
            if (l_entry.contains("child") && l_entry.contains("parent"))
            {
                int l_child  = l_entry["child"].get<int>();
                int l_parent = l_entry["parent"].get<int>();
                m_sceneEdit->SetEntityParent(l_child, l_parent);
            }
        }
    }
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

    // Full-viewport dockspace (passthrough so scene renders behind)
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
                                 ImGuiDockNodeFlags_PassthruCentralNode);

    // Clear tutorial highlight regions registered last frame
    m_tutorialPanel.ClearRegions();

    // Menu bar and tutorial panel are universal across all tabs
    m_topBar.Draw();

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

    // Theme settings panel (dockable window).
    m_themeSettingsPanel.DrawAsWindow();

    // Status bar at the bottom of the viewport
    DrawStatusBar();

    // Rendering
    ImGui::PopFont();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Flush deferred game-UI draw commands on top of all editor panels.
    GetUIRenderer().Flush();

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

    // Middle mouse button pan — anchor world point under cursor
    RE::IO::MouseInfo l_lastMouseInfo = m_engineContents.io->GetLastFrameMouseInfo();
    if (l_mouseInfo.MMBDown && !l_lastMouseInfo.MMBDown)
    {
        // MMB just pressed: record world point under mouse
        Vector3 l_anchor = m_sceneEdit->m_camera->ScreenToWorldPoint(l_mouseInfo.position);
        m_sceneEdit->m_panAnchorWorld = Vector2(l_anchor.x, l_anchor.y);
        m_sceneEdit->m_isPanningCamera = true;
    }
    else if (l_mouseInfo.MMBDown && m_sceneEdit->m_isPanningCamera)
    {
        // MMB held: shift camera so anchor stays under mouse
        Vector3 l_currentAtMouse = m_sceneEdit->m_camera->ScreenToWorldPoint(l_mouseInfo.position);
        Vector3 l_camPos = m_sceneEdit->m_camera->GetPosition();
        l_camPos.x -= l_currentAtMouse.x - m_sceneEdit->m_panAnchorWorld.x;
        l_camPos.y -= l_currentAtMouse.y - m_sceneEdit->m_panAnchorWorld.y;
        m_sceneEdit->m_camera->SetPosition(l_camPos);
    }
    else if (!l_mouseInfo.MMBDown)
    {
        m_sceneEdit->m_isPanningCamera = false;
    }

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

                    // Snapshot all selected entities for multi-entity undo
                    m_sceneEdit->m_dragStartTransforms.clear();
                    for (int l_id : m_sceneEdit->m_selectedEntities)
                    {
                        sol::table l_tf = l_scene->GetRegistry()
                            .GetEntityAttributes(l_id)
                            .raw_get<sol::table>("Transform");
                        m_sceneEdit->m_dragStartTransforms[l_id] = {
                            l_tf.raw_get<Vector2>("Position"),
                            l_tf.raw_get<Vector2>("Scale"),
                            l_tf.raw_get<float>("Rotation")
                        };
                    }
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

                bool l_shiftHeld = ImGui::GetIO().KeyShift;

                if (l_hitEntity > -1)
                {
                    if (l_shiftHeld)
                    {
                        // Shift+click: toggle entity in multi-selection
                        m_sceneEdit->ToggleEntitySelection(l_hitEntity);
                    }
                    else
                    {
                        // Normal click: select only this entity
                        m_sceneEdit->SelectEntity(l_hitEntity);
                        ImVec2 l_panelSize = m_sceneEdit->m_propertiesPanel.GetSize();

                        RE::Core::EntityRegistry& l_registry = l_scene->GetRegistry();

                        Vector2 l_entityWorldPos = l_registry.GetEntityAttributes(l_hitEntity).traverse_raw_get<Vector2>("Transform", "Position");

                        Vector2 l_entityScreenPos = m_sceneEdit->m_camera->WorldToScreenPoint(l_entityWorldPos);

                        float l_screenH = m_engineContents.io->GetWindow().lock()->GetScreenSize().y;
                        m_sceneEdit->m_propertiesPanel.SetPosition(ImVec2(l_entityScreenPos.x + l_panelSize.x * 0.25f, l_screenH - l_entityScreenPos.y - l_panelSize.y * 0.25f));
                    }

                    // Start entity drag for all selected entities
                    if (m_sceneEdit->IsEntitySelected(l_hitEntity))
                    {
                        m_sceneEdit->m_isDraggingEntity = true;
                        m_sceneEdit->m_dragStartWorldPos = Vector2(l_mouseWorldPos.x, l_mouseWorldPos.y);
                        m_sceneEdit->m_dragStartEntityPos = l_scene->GetRegistry()
                            .GetEntityAttributes(m_sceneEdit->m_selectedEntityId)
                            .traverse_raw_get<Vector2>("Transform", "Position");

                        // Snapshot all selected entities for multi-entity undo
                        m_sceneEdit->m_dragStartTransforms.clear();
                        for (int l_id : m_sceneEdit->m_selectedEntities)
                        {
                            sol::table l_tf = l_scene->GetRegistry()
                                .GetEntityAttributes(l_id)
                                .raw_get<sol::table>("Transform");
                            m_sceneEdit->m_dragStartTransforms[l_id] = {
                                l_tf.raw_get<Vector2>("Position"),
                                l_tf.raw_get<Vector2>("Scale"),
                                l_tf.raw_get<float>("Rotation")
                            };
                        }
                    }
                }
                else
                {
                    if (!l_shiftHeld)
                    {
                        m_sceneEdit->ClearSelection();
                    }
                    // Start drag-select rectangle
                    float l_screenH = m_engineContents.io->GetWindow().lock()->GetScreenSize().y;
                    m_sceneEdit->m_isDraggingRect = true;
                    m_sceneEdit->m_dragRectStart = Vector2(l_mouseInfo.position.x, l_screenH - l_mouseInfo.position.y);
                    m_sceneEdit->m_dragRectEnd = m_sceneEdit->m_dragRectStart;
                }

                //RE::Log::Message("Clicked entity: " + std::to_string(l_hitEntity));
            }
        }

        // LMB held: drag entity / gizmo / selection rectangle
        if (l_mouseInfo.LMBDown && m_sceneEdit->m_isDraggingRect)
        {
            // Update drag-select rectangle end point
            float l_screenH = m_engineContents.io->GetWindow().lock()->GetScreenSize().y;
            m_sceneEdit->m_dragRectEnd = Vector2(l_mouseInfo.position.x, l_screenH - l_mouseInfo.position.y);
        }

        if (l_mouseInfo.LMBDown && m_sceneEdit->m_isDraggingEntity && m_sceneEdit->m_selectedEntityId >= 0)
        {
            Vector3 l_currentWorld = m_sceneEdit->m_camera->ScreenToWorldPoint(l_mouseInfo.position);
            Vector2 l_delta(
                l_currentWorld.x - m_sceneEdit->m_dragStartWorldPos.x,
                l_currentWorld.y - m_sceneEdit->m_dragStartWorldPos.y
            );

            auto l_scene = m_engineContents.core->GetScene().lock();
            SceneSettings& l_ss = m_sceneEdit->GetSceneSettings();

            if (m_sceneEdit->m_isDraggingGizmo)
            {
                Gizmo::Axis l_axis = m_sceneEdit->m_activeGizmoAxis;

                // Apply gizmo transform to all selected entities
                for (int l_id : m_sceneEdit->m_selectedEntities)
                {
                    sol::table l_transform = l_scene->GetRegistry()
                        .GetEntityAttributes(l_id)
                        .raw_get<sol::table>("Transform");
                    auto l_startIt = m_sceneEdit->m_dragStartTransforms.find(l_id);
                    if (l_startIt == m_sceneEdit->m_dragStartTransforms.end()) continue;
                    auto& l_start = l_startIt->second;

                    if (l_axis == Gizmo::Axis::Rotate)
                    {
                        Vector2 l_entityPos = m_sceneEdit->m_dragStartEntityPos;

                        float l_angle = std::atan2(
                            l_currentWorld.y - l_entityPos.y,
                            l_currentWorld.x - l_entityPos.x);
                        
                        float l_startAngle = std::atan2(
                            m_sceneEdit->m_dragStartWorldPos.y - l_entityPos.y,
                            m_sceneEdit->m_dragStartWorldPos.x - l_entityPos.x);
                        
                        float l_deltaAngle = l_angle - l_startAngle;
                        l_deltaAngle = std::atan2(std::sin(l_deltaAngle), std::cos(l_deltaAngle));
                        
                        float l_deltaDeg = l_deltaAngle * (180.0f / RE::Math::PI());
                        float l_newRot = l_start.rotation + l_deltaDeg;
                        
                        if(l_newRot > 180.0f) l_newRot -= 360.0f;
                        else if(l_newRot <= -180.0f) l_newRot += 360.0f;

                        l_transform.raw_set("Rotation", l_newRot);
                    }
                    else if (l_axis == Gizmo::Axis::ScaleX || l_axis == Gizmo::Axis::ScaleY || l_axis == Gizmo::Axis::ScaleXY)
                    {
                        Vector2 l_constrained = Gizmo::ConstrainDelta(l_axis, l_delta);
                        Vector2 l_newScale = l_start.scale + l_constrained;
                        
                        if (l_newScale.x < 0.01f) l_newScale.x = 0.01f;
                        if (l_newScale.y < 0.01f) l_newScale.y = 0.01f;
                        l_transform.raw_set("Scale", l_newScale);
                    }
                    else
                    {
                        Vector2 l_constrained = Gizmo::ConstrainDelta(l_axis, l_delta);
                        Vector2 l_newPos = l_start.position + l_constrained;
                        if (l_ss.snapEnabled)
                            l_newPos = Gizmo::Snap(l_newPos, l_ss.snapGridSize);
                        l_transform.raw_set("Position", l_newPos);
                    }
                }
            }
            else
            {
                // Free drag — move all selected entities
                for (int l_id : m_sceneEdit->m_selectedEntities)
                {
                    auto l_startIt = m_sceneEdit->m_dragStartTransforms.find(l_id);
                    if (l_startIt == m_sceneEdit->m_dragStartTransforms.end()) continue;
                    auto& l_start = l_startIt->second;

                    sol::table l_transform = l_scene->GetRegistry()
                        .GetEntityAttributes(l_id)
                        .raw_get<sol::table>("Transform");

                    Vector2 l_newPos = l_start.position + l_delta;
                    if (l_ss.snapEnabled)
                        l_newPos = Gizmo::Snap(l_newPos, l_ss.snapGridSize);
                    l_transform.raw_set("Position", l_newPos);
                }
            }
        }

        // LMB released: end drag, push undo
        if (!l_mouseInfo.LMBDown && m_sceneEdit->m_isDraggingEntity)
        {
            m_sceneEdit->m_isDraggingEntity = false;
            m_sceneEdit->m_isDraggingGizmo  = false;
            m_sceneEdit->m_activeGizmoAxis  = Gizmo::Axis::None;

            if (!m_sceneEdit->m_selectedEntities.empty())
            {
                // Build per-entity old & new transform maps for undo
                using DragStart = SceneEditTab::EntityDragStart;
                auto l_scene = m_engineContents.core->GetScene().lock();

                struct TransformPair { DragStart oldT; DragStart newT; };
                auto l_changes = std::make_shared<std::unordered_map<int, TransformPair>>();

                for (int l_id : m_sceneEdit->m_selectedEntities)
                {
                    auto l_startIt = m_sceneEdit->m_dragStartTransforms.find(l_id);
                    if (l_startIt == m_sceneEdit->m_dragStartTransforms.end()) continue;

                    sol::table l_tf = l_scene->GetRegistry()
                        .GetEntityAttributes(l_id)
                        .raw_get<sol::table>("Transform");

                    DragStart l_new{
                        l_tf.raw_get<Vector2>("Position"),
                        l_tf.raw_get<Vector2>("Scale"),
                        l_tf.raw_get<float>("Rotation")
                    };

                    bool l_changed = (l_new.position.x != l_startIt->second.position.x ||
                                      l_new.position.y != l_startIt->second.position.y ||
                                      l_new.scale.x    != l_startIt->second.scale.x ||
                                      l_new.scale.y    != l_startIt->second.scale.y ||
                                      l_new.rotation   != l_startIt->second.rotation);

                    if (l_changed)
                        (*l_changes)[l_id] = { l_startIt->second, l_new };
                }

                if (!l_changes->empty())
                {
                    Editor* l_self = this;

                    m_undoManager.PushCommand(
                        std::make_unique<LambdaCommand>(
                            "Transform Entity",
                            [l_self, l_changes]() {
                                auto l_sc = l_self->GetEngineContents().core->GetScene().lock();
                                for (auto& [l_id, l_pair] : *l_changes)
                                {
                                    sol::table l_tf = l_sc->GetRegistry()
                                        .GetEntityAttributes(l_id)
                                        .raw_get<sol::table>("Transform");
                                    l_tf.raw_set("Position", l_pair.newT.position);
                                    l_tf.raw_set("Scale",    l_pair.newT.scale);
                                    l_tf.raw_set("Rotation", l_pair.newT.rotation);
                                }
                            },
                            [l_self, l_changes]() {
                                auto l_sc = l_self->GetEngineContents().core->GetScene().lock();
                                for (auto& [l_id, l_pair] : *l_changes)
                                {
                                    sol::table l_tf = l_sc->GetRegistry()
                                        .GetEntityAttributes(l_id)
                                        .raw_get<sol::table>("Transform");
                                    l_tf.raw_set("Position", l_pair.oldT.position);
                                    l_tf.raw_set("Scale",    l_pair.oldT.scale);
                                    l_tf.raw_set("Rotation", l_pair.oldT.rotation);
                                }
                            }
                        )
                    );
                }
            }
        }

        // LMB released: finish drag-select rectangle
        if (!l_mouseInfo.LMBDown && m_sceneEdit->m_isDraggingRect)
        {
            m_sceneEdit->m_isDraggingRect = false;

            // Convert rectangle corners to world space and select all entities inside
            float l_screenH = m_engineContents.io->GetWindow().lock()->GetScreenSize().y;
            // m_dragRectStart/End are in ImGui screen space (Y=0 at top); convert back to SDL (Y=0 at bottom)
            Vector2 l_sdlStart(m_sceneEdit->m_dragRectStart.x, l_screenH - m_sceneEdit->m_dragRectStart.y);
            Vector2 l_sdlEnd(m_sceneEdit->m_dragRectEnd.x, l_screenH - m_sceneEdit->m_dragRectEnd.y);

            Vector3 l_worldMin3 = m_sceneEdit->m_camera->ScreenToWorldPoint(
                Vector2(std::min(l_sdlStart.x, l_sdlEnd.x), std::min(l_sdlStart.y, l_sdlEnd.y)));
            Vector3 l_worldMax3 = m_sceneEdit->m_camera->ScreenToWorldPoint(
                Vector2(std::max(l_sdlStart.x, l_sdlEnd.x), std::max(l_sdlStart.y, l_sdlEnd.y)));

            float l_minX = std::min(l_worldMin3.x, l_worldMax3.x);
            float l_maxX = std::max(l_worldMin3.x, l_worldMax3.x);
            float l_minY = std::min(l_worldMin3.y, l_worldMax3.y);
            float l_maxY = std::max(l_worldMin3.y, l_worldMax3.y);

            auto l_scene = m_engineContents.core->GetScene().lock();
            RE::Core::EntityRegistry& l_reg = l_scene->GetRegistry();
            std::vector<int> l_allIds = l_reg.GetAllRegisteredIds();

            bool l_shiftHeld = ImGui::GetIO().KeyShift;
            if (!l_shiftHeld)
                m_sceneEdit->m_selectedEntities.clear();

            for (int l_id : l_allIds)
            {
                Vector2 l_pos = l_reg.GetEntityAttributes(l_id)
                    .traverse_raw_get<Vector2>("Transform", "Position");
                if (l_pos.x >= l_minX && l_pos.x <= l_maxX &&
                    l_pos.y >= l_minY && l_pos.y <= l_maxY)
                {
                    if (!m_sceneEdit->IsEntitySelected(l_id))
                        m_sceneEdit->m_selectedEntities.push_back(l_id);
                }
            }

            // Update primary selection
            if (!m_sceneEdit->m_selectedEntities.empty())
            {
                m_sceneEdit->m_selectedEntityId = m_sceneEdit->m_selectedEntities.back();
                m_sceneEdit->m_propertiesPanel.SetShown(true);
            }
            else
            {
                m_sceneEdit->m_selectedEntityId = -1;
                m_sceneEdit->m_propertiesPanel.SetShown(false);
            }
        }

        m_sceneEdit->m_camera->SetCameraWidth(m_sceneEdit->m_camera->GetCameraWidth() + l_mouseInfo.deltaScroll);
    }

    if (!ImGui::GetIO().WantTextInput)
    {
        // Gizmo mode shortcuts (T/R/S — mnemonic)
        SceneSettings& l_ss = m_sceneEdit->GetSceneSettings();
        if (m_engineContents.io->GetKeyDownThisFrame('t'))
            l_ss.gizmoMode = GizmoMode::Translate;
        if (m_engineContents.io->GetKeyDownThisFrame('r'))
            l_ss.gizmoMode = GizmoMode::Rotate;


        if (!ImGui::GetIO().KeyCtrl && m_engineContents.io->GetKeyDownThisFrame('s'))
            l_ss.gizmoMode = GizmoMode::Scale;

        if (!ImGui::GetIO().KeyCtrl && m_engineContents.io->GetKeyDownThisFrame('e'))
        {
            m_sceneEdit->m_entityPanel.SetShown(!m_sceneEdit->m_entityPanel.IsShown());
        }

        if (!ImGui::GetIO().KeyCtrl && m_engineContents.io->GetKeyDownThisFrame('c'))
        {
            m_sceneEdit->m_categoryPanel.SetShown(!m_sceneEdit->m_categoryPanel.IsShown());
        }

        if (m_engineContents.io->GetKeyDownThisFrame(' '))
        {
            m_sceneEdit->m_assetBrowserPanel.SetShown(!m_sceneEdit->m_assetBrowserPanel.IsShown());
        }

        if (m_engineContents.io->GetKeyDownThisFrame('`'))
        {
            m_sceneEdit->m_consolePanel.SetShown(!m_sceneEdit->m_consolePanel.IsShown());
        }

        // ESC input to close all panels and context menus
        if (m_engineContents.io->GetKeyDownThisFrame((char)27))
        {
            m_sceneEdit->CloseAllPanels();
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

        // Select all entities (Ctrl+A)
        if (ImGui::GetIO().KeyCtrl && m_engineContents.io->GetKeyDownThisFrame('a'))
        {
            m_sceneEdit->SelectAllEntities();
        }

        // Copy/Paste/Duplicate shortcuts
        if (ImGui::GetIO().KeyCtrl && m_engineContents.io->GetKeyDownThisFrame('c'))
        {
            CopySelectedEntities();
        }
        if (ImGui::GetIO().KeyCtrl && m_engineContents.io->GetKeyDownThisFrame('v'))
        {
            PasteEntities();
        }
        if (ImGui::GetIO().KeyCtrl && m_engineContents.io->GetKeyDownThisFrame('d'))
        {
            DuplicateEntities();
        }

        // Quick-Save (Ctrl+S)
        if (ImGui::GetIO().KeyCtrl && m_engineContents.io->GetKeyDownThisFrame('s'))
        {
            if (!m_currentScenePath.empty())
            {
                SceneSerializer::SaveToFile(m_currentScenePath, m_engineContents,
                                            m_sceneEdit->GetSceneSettings());
                SaveProjectInfo();
                RE::Log::Message("Scene saved: " + m_currentScenePath);
            }
            else
            {
                m_topBar.OpenSaveDialog();
            }
        }

        // Delete selected entities (Delete key)
        if (m_engineContents.io->GetKeyDownThisFrame(127) && !m_sceneEdit->m_selectedEntities.empty())
        {
            // Delete all selected entities
            std::vector<int> l_toDelete = m_sceneEdit->m_selectedEntities;
            for (int l_id : l_toDelete)
            {
                m_sceneEdit->m_entityPanel.RemoveEntity(l_id);
            }
            m_sceneEdit->ClearSelection();
        }

        // Tab input to switch between scene and text edit
        if(m_engineContents.io->GetKeyDownThisFrame('\t'))
        {
            if(m_state == State::SceneEdit)
            {
                m_sceneEdit->CloseAllPanels();
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

void Editor::CopySelectedEntities()
{
    m_clipboard.clear();
    auto l_scene = m_engineContents.core->GetScene().lock();
    if (!l_scene) return;

    for (int l_id : m_sceneEdit->GetSelectedEntities())
    {
        EntitySnapshot l_snap;
        l_snap.name = l_scene->GetRegistry().GetEntityName(l_id);
        sol::table l_attrs = l_scene->GetRegistry().GetEntityAttributes(l_id);
        sol::state_view l_lua(l_attrs.lua_state());
        for (auto& l_catPair : l_attrs)
        {
            std::string l_catName = l_catPair.first.as<std::string>();
            sol::table l_fields = l_catPair.second.as<sol::table>();
            std::vector<std::pair<std::string, sol::object>> l_fieldSnap;
            for (auto& l_fp : l_fields)
            {
                sol::object l_val = l_fp.second;
                // Deep-copy primitive and Vector types so the clipboard
                // is independent of the source entity's Lua table.
                if (l_val.is<int>())
                    l_val = sol::make_object(l_lua, l_val.as<int>());
                else if (l_val.is<float>())
                    l_val = sol::make_object(l_lua, l_val.as<float>());
                else if (l_val.is<bool>())
                    l_val = sol::make_object(l_lua, l_val.as<bool>());
                else if (l_val.is<std::string>())
                    l_val = sol::make_object(l_lua, l_val.as<std::string>());
                else if (l_val.is<Vector2>())
                    l_val = sol::make_object(l_lua, l_val.as<Vector2>());
                else if (l_val.is<Vector3>())
                    l_val = sol::make_object(l_lua, l_val.as<Vector3>());
                else if (l_val.is<Vector4>())
                    l_val = sol::make_object(l_lua, l_val.as<Vector4>());
                l_fieldSnap.emplace_back(l_fp.first.as<std::string>(), l_val);
            }
            l_snap.categories.emplace_back(l_catName, std::move(l_fieldSnap));
        }
        m_clipboard.push_back(std::move(l_snap));
    }

    RE::Log::Message("Copied " + std::to_string(m_clipboard.size()) + " entities");
}

void Editor::PasteEntities()
{
    if (m_clipboard.empty()) return;

    auto l_scene = m_engineContents.core->GetScene().lock();
    if (!l_scene) return;

    m_sceneEdit->ClearSelection();
    std::vector<int> l_newIds;

    for (auto& l_snap : m_clipboard)
    {
        int l_newId = l_scene->AddEntity();

        for (auto& [l_catName, l_fields] : l_snap.categories)
        {
            if (l_catName != "Transform")
                l_scene->AddToCategory(l_newId, l_catName);
        }

        sol::table l_attrs = l_scene->GetRegistry().GetEntityAttributes(l_newId);
        for (auto& [l_catName, l_fields] : l_snap.categories)
        {
            sol::object l_catObj = l_attrs.raw_get<sol::object>(l_catName.c_str());
            if (!l_catObj.valid() || l_catObj.get_type() != sol::type::table)
                continue;
            sol::table l_catAttrs = l_catObj.as<sol::table>();
            for (auto& [l_fname, l_fval] : l_fields)
                l_catAttrs[l_fname] = l_fval;
        }

        // Offset position so paste isn't exactly on top of original
        sol::table l_transform = l_attrs.raw_get<sol::table>("Transform");
        Vector2 l_pos = l_transform.raw_get<Vector2>("Position");
        l_transform.raw_set("Position", Vector2(l_pos.x + 1.0f, l_pos.y + 1.0f));

        if (!l_snap.name.empty())
            l_scene->RenameEntity(l_newId, l_snap.name + " (Copy)");

        l_newIds.push_back(l_newId);
    }

    m_sceneEdit->GetEntityPanel().RefreshEntityList();
    for (int l_id : l_newIds)
        m_sceneEdit->m_selectedEntities.push_back(l_id);
    if (!l_newIds.empty())
    {
        m_sceneEdit->m_selectedEntityId = l_newIds.front();
        m_sceneEdit->m_propertiesPanel.SetShown(true);
    }

    RE::Log::Message("Pasted " + std::to_string(l_newIds.size()) + " entities");
}

void Editor::DuplicateEntities()
{
    // Duplicate = copy then paste in one step
    CopySelectedEntities();
    PasteEntities();
}

void Editor::DrawStatusBar()
{
    ImGuiIO& l_io = ImGui::GetIO();
    float l_barHeight = ImGui::GetFrameHeight();

    ImGui::SetNextWindowPos(ImVec2(0, l_io.DisplaySize.y - l_barHeight));
    ImGui::SetNextWindowSize(ImVec2(l_io.DisplaySize.x, l_barHeight));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));

    if (ImGui::Begin("##StatusBar", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoDocking))
    {
        // Play/Edit indicator
        if (m_sceneEdit->IsGameRunning())
        {
            ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.2f, 1.0f), m_sceneEdit->IsGamePaused() ? "PAUSED" : "PLAYING");
        }
        else
        {
            ImGui::TextColored(ImVec4(0.5f, 0.7f, 1.0f, 1.0f), "EDIT");
        }

        ImGui::SameLine(0.0f, 20.0f);
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine(0.0f, 20.0f);

        auto l_scene = m_engineContents.core->GetScene().lock();

        int l_entityCount = l_scene->GetRegistry().GetEntityCount();
        ImGui::Text("Entities: %d", l_entityCount);

        ImGui::SameLine(0.0f, 20.0f);
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine(0.0f, 20.0f);

        // Selected entity info
        const auto& l_sel = m_sceneEdit->GetSelectedEntities();
        if (l_sel.empty())
        {
            ImGui::TextDisabled("No selection");
        }
        else if (l_sel.size() == 1)
        {
            std::string l_name = l_scene ? l_scene->GetRegistry().GetEntityName(l_sel[0]) : "";
            ImGui::Text("Selected: %s (#%d)", l_name.c_str(), l_sel[0]);
        }
        else
        {
            ImGui::Text("Selected: %d entities", (int)l_sel.size());
        }

        ImGui::SameLine(0.0f, 20.0f);
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine(0.0f, 20.0f);

        // Tool mode
        const SceneSettings& l_ss = m_sceneEdit->GetSceneSettings();
        const char* l_modeName = "Translate";
        if (l_ss.gizmoMode == GizmoMode::Rotate)  l_modeName = "Rotate";
        if (l_ss.gizmoMode == GizmoMode::Scale)   l_modeName = "Scale";
        ImGui::Text("Tool: %s", l_modeName);

        ImGui::SameLine(0.0f, 20.0f);
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine(0.0f, 20.0f);

        // Snap state
        if (l_ss.snapEnabled)
            ImGui::Text("Snap: %.1f", l_ss.snapGridSize);
        else
            ImGui::TextDisabled("Snap: Off");

        // FPS on the right side
        ImGui::SameLine(l_io.DisplaySize.x - 80.0f);
        ImGui::Text("%.0f FPS", l_io.Framerate);
    }
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

void Editor::SaveLayout(int _slot)
{
    if (_slot < 0 || _slot >= 3) return;

    const char* l_data = ImGui::SaveIniSettingsToMemory();
    m_savedLayouts[_slot] = std::string(l_data);

    // Persist to file
    std::string l_path = m_project.GetRootPath() + "/layout_" + std::to_string(_slot) + ".ini";
    std::ofstream l_file(l_path);
    if (l_file.is_open())
    {
        l_file << m_savedLayouts[_slot];
        l_file.close();
    }

    RE::Log::Message("Saved layout to slot " + std::to_string(_slot + 1));
}

void Editor::LoadLayout(int _slot)
{
    if (_slot < 0 || _slot >= 3) return;

    if (m_savedLayouts[_slot].empty())
    {
        // Try loading from file
        std::string l_path = m_project.GetRootPath() + "/layout_" + std::to_string(_slot) + ".ini";
        std::ifstream l_file(l_path);
        if (l_file.is_open())
        {
            std::stringstream l_buf;
            l_buf << l_file.rdbuf();
            m_savedLayouts[_slot] = l_buf.str();
        }
    }

    if (!m_savedLayouts[_slot].empty())
    {
        ImGui::LoadIniSettingsFromMemory(m_savedLayouts[_slot].c_str(), m_savedLayouts[_slot].size());
        RE::Log::Message("Loaded layout from slot " + std::to_string(_slot + 1));
    }
}

void Editor::LoadSavedLayouts()
{
    for (int i = 0; i < 3; ++i)
    {
        std::string l_path = m_project.GetRootPath() + "/layout_" + std::to_string(i) + ".ini";
        std::ifstream l_file(l_path);
        if (l_file.is_open())
        {
            std::stringstream l_buf;
            l_buf << l_file.rdbuf();
            m_savedLayouts[i] = l_buf.str();
        }
    }
}