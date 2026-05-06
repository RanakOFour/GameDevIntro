#include "Editor/Tabs/SceneEditTab.h"
#include "Editor/Scene/BuiltIn/EditorAssets.h"
#include "Editor/Scene/SceneSerializer.h"
#include "Editor/Core/StateRegistry.h"
#include "Editor/Panels/Scene/SceneSettingsPanel.h"
#include "Editor/Scene/BuiltIn/BuiltinRules.h"
#include "Editor/Panels/Scene/AssetBrowserPanel.h"

#include <filesystem>

#include "RanakEngine/Assets.h"
#include "RanakEngine/Physics/PhysicsManager.h"


SceneEditTab::SceneEditTab(Editor& _editor)
: m_editor(_editor)
, m_entityPanel(_editor)
, m_propertiesPanel(_editor)
, m_categoryPanel(_editor)
, m_rulesPanel(_editor)
, m_cameraPanel(_editor)
, m_selectedEntityId(-1)
, m_isGameRunning(false)
, m_lastFrameTime(SDL_GetPerformanceCounter())
, m_sceneSettings()
, m_settingsPanel(_editor, &m_sceneSettings)
, m_consolePanel(_editor)
, m_assetBrowserPanel(_editor)
{
	auto l_engineContents = _editor.GetEngineContents();

	m_scene = l_engineContents.core->GetScene();
	m_camera = l_engineContents.core->GetCamera().lock();
	m_window = l_engineContents.io->GetWindow().lock();

    std::filesystem::path l_gridFragPath = RE::Asset::GetTempDir() / "Shaders" / "grid_shader.fs";
    std::filesystem::path l_gridVertPath = RE::Asset::GetTempDir() / "Shaders" / "grid_shader.vs";

    if(!std::filesystem::exists(l_gridFragPath) || !std::filesystem::exists(l_gridVertPath))
    {
        RE::Log::Message("Grid shader files not found at expected paths:\n" + l_gridFragPath.string() + "\n" + l_gridVertPath.string());
        // Write default grid shader to temp directory so it can be loaded like a normal shader asset.
        std::filesystem::create_directories(l_gridFragPath.parent_path());
        std::ofstream l_fileWriter(l_gridFragPath);
        l_fileWriter.write(EditorAssets::DefaultGridShaderFragment, EditorAssets::DefaultGridShaderFragmentSize);
        l_fileWriter.close();

        l_fileWriter.open(l_gridVertPath);
        l_fileWriter.write(EditorAssets::DefaultGridShaderVertex, EditorAssets::DefaultGridShaderVertexSize);
        l_fileWriter.close();
    }

	m_gridShader = l_engineContents.resources->Load<RE::Asset::Shader>(l_gridFragPath.string() + ";" + l_gridVertPath.string()).lock();
    glGenVertexArrays(1, &m_dummyGridVAO);

    // Register SceneEditTab conditions and actions in the shared StateRegistry.
    SceneEditTab* l_self = this;
    StateRegistry& l_reg = _editor.GetStateRegistry();
    l_reg.RegisterCondition("game_running",    [l_self]{ return  l_self->m_isGameRunning; });
    l_reg.RegisterCondition("game_stopped",    [l_self]{ return !l_self->m_isGameRunning; });
    l_reg.RegisterCondition("game_paused",     [l_self]{ return  l_self->m_isGamePaused; });
    l_reg.RegisterCondition("entity_selected", [l_self]{ return !l_self->m_selectedEntities.empty(); });
    l_reg.RegisterAction("game_run",    [l_self]{ l_self->Run(); });
    l_reg.RegisterAction("game_stop",   [l_self]{ l_self->Stop(); });
    l_reg.RegisterAction("game_pause",  [l_self]{ l_self->Pause(); });
    l_reg.RegisterAction("game_resume", [l_self]{ l_self->Resume(); });

    m_assetBrowserPanel.SetRootPath(_editor.GetProject().GetRootPath());

    // Load editor icons for asset browser (user provides these textures)
    std::string l_folderIcon = RE::Asset::GetTempDir() / "Textures" / "folder_icon.png";
    std::string l_fileIcon   = RE::Asset::GetTempDir() / "Textures" / "file_icon.png";
    if (std::filesystem::exists(l_folderIcon) && std::filesystem::exists(l_fileIcon))
    {
        m_assetBrowserPanel.LoadIcons(l_folderIcon, l_fileIcon);
    }
}

SceneEditTab::~SceneEditTab()
{
	glDeleteVertexArrays(1, &m_dummyGridVAO);

	m_camera.reset();
    m_gridShader.reset();
}

void SceneEditTab::DrawEditorUI()
{
    // When a non-interactive tutorial step is active, suppress the context menu
    // and disable all editor panel widgets so the user can only interact with
    // the tutorial window itself.
    TutorialPanel& l_tutorial = m_editor.GetTutorialPanel();
    const bool l_tutLocked = l_tutorial.IsActive() && !l_tutorial.IsStepInteractive();

    if (!l_tutLocked)
        DrawContextMenu();

    // If the current tutorial step highlights a specific panel, ensure it is
    // visible before Draw() is called so FindWindowByName can locate it.
    const std::string& l_highlightKey = l_tutorial.GetCurrentHighlightKey();
    if(l_highlightKey == "Entity List")
    {
        m_entityPanel.SetShown(true);
    }
    else if (l_highlightKey == "Categories")
    {
        m_categoryPanel.SetShown(true);
    }
    else if (l_highlightKey == "Rules")
    {
        m_rulesPanel.SetShown(true);
    }
    else if (l_highlightKey == "Entity Properties")
    {
        m_propertiesPanel.SetShown(true);
    }
    else if (l_highlightKey == "Camera")
    {
        m_cameraPanel.SetShown(true);
    }
    else if (l_highlightKey == "Console")
    {
        m_consolePanel.SetShown(true);
    }

    if (l_tutLocked)
    {
        ImGui::BeginDisabled();
    }

    m_entityPanel.DrawAsWindow();
    m_categoryPanel.DrawAsWindow();
    m_categoryPanel.DrawLoadCategoryDialog();
    m_rulesPanel.DrawAsWindow();
    m_rulesPanel.DrawLoadRuleDialog();
    m_cameraPanel.DrawAsWindow();
    m_propertiesPanel.DrawAsWindow();
    m_settingsPanel.DrawAsWindow();
    m_consolePanel.DrawAsWindow();
    m_assetBrowserPanel.DrawAsWindow();

    if (l_tutLocked)
    {
        ImGui::EndDisabled();
    }
}

void SceneEditTab::DrawContextMenu()
{
    auto l_scene = m_scene.lock();
	// Context menu
    if(ImGui::BeginPopupContextVoid("ContextMenu", ImGuiPopupFlags_MouseButtonRight))
    {
        ImVec2 l_buttonSize(170, 25);
        if(ImGui::Button("Create Entity", l_buttonSize))
        {	
			Vector2 l_mousePos = m_editor.GetEngineContents().io->GetMouseInfo().position;
            Vector3 l_mousePosWorld = m_editor.GetEngineContents().core->ScreenToWorldPoint(l_mousePos);
            Vector2 l_entityPos(l_mousePosWorld.x, l_mousePosWorld.y);

            m_entityPanel.AddEntity();

            sol::table l_entityTransform = l_scene->GetRegistry()
                                            .GetEntityAttributes(m_selectedEntityId)
                                            .raw_get<sol::table>("Transform");

            l_entityTransform.raw_set("Position", l_entityPos);

            m_propertiesPanel.SetShown(true);
            ImVec2 l_panelSize = m_propertiesPanel.GetSize();

            float l_screenH = m_editor.GetEngineContents().io->GetWindow().lock()->GetScreenSize().y;
            m_propertiesPanel.SetPosition(ImVec2(l_mousePos.x + l_panelSize.x * 0.25f, l_screenH - l_mousePos.y - l_panelSize.y * 0.25f));
        }

        if(!m_selectedEntities.empty())
        {
            std::string l_deleteLabel = m_selectedEntities.size() > 1
                ? "Delete " + std::to_string(m_selectedEntities.size()) + " Entities"
                : "Delete Entity";
            if(ImGui::Button(l_deleteLabel.c_str(), l_buttonSize))
            {
                for (int l_id : m_selectedEntities)
                {    
                    l_scene->RemoveEntity(l_id);
                }
                ClearSelection();
            }
        }

        if(!m_cameraPanel.IsShown())
        {
            if(ImGui::Button("Show Camera Settings", l_buttonSize))
            {
                m_cameraPanel.SetShown(true);
            }
        }

        if(!m_entityPanel.IsShown())
        {
            if(ImGui::Button("Show Entity List", l_buttonSize))
            {
                m_entityPanel.SetShown(true);
            }
        }

        if(!m_categoryPanel.IsShown())
        {
            if(ImGui::Button("Show Category List", l_buttonSize))
            {
                m_categoryPanel.SetShown(true);
            }
        }

        if(!m_rulesPanel.IsShown())
        {
            if(ImGui::Button("Show Rules List", l_buttonSize))
            {
                m_rulesPanel.SetShown(true);
            }
        }

        if(!m_assetBrowserPanel.IsShown())
        {
            if(ImGui::Button("Show Asset Browser", l_buttonSize))
            {
                m_assetBrowserPanel.SetShown(true);
            }
        }

        ImGui::EndPopup();
    }
}

void SceneEditTab::Draw()
{
    // Compute delta time
    Uint64 l_now = SDL_GetPerformanceCounter();
    float l_dt = (float)(l_now - m_lastFrameTime) / (float)SDL_GetPerformanceFrequency();
    m_lastFrameTime = l_now;

    ImGuiIO& l_io = ImGui::GetIO();
    const float l_playBtnW = 64.0f;
    const float l_stopBtnW = 64.0f;
    const float l_pauseBtnW = 80.0f;
    const float l_spacing = 8.0f;
    const float l_toolW = m_isGameRunning
        ? l_stopBtnW + l_spacing + l_pauseBtnW
        : l_playBtnW;
    ImGui::SetNextWindowPos(ImVec2((l_io.DisplaySize.x - l_toolW) * 0.5f, 24.0f));
    ImGui::SetNextWindowSize(ImVec2(l_toolW + 16.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.75f);
    if(ImGui::Begin("##PlayToolbar", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoBringToFrontOnFocus))
    {
        if (m_isGameRunning)
        {
            if (ImGui::Button("Stop", ImVec2(l_stopBtnW, 0.0f)))
            {
                Stop();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Stop simulation and restore scene");
            ImGui::SameLine(0.0f, l_spacing);
            std::string l_pauseLabel = m_isGamePaused ? "Resume" : "Pause";
            if (ImGui::Button(l_pauseLabel.c_str(), ImVec2(l_pauseBtnW, 0.0f)))
            {
                if (m_isGamePaused) Resume(); else Pause();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip(m_isGamePaused ? "Resume simulation" : "Pause simulation");
        }
        else
        {
            if (ImGui::Button("Play", ImVec2(l_playBtnW, 0.0f)))
            {
                Run();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Start simulation");
        }
    }
    ImGui::End();

    auto l_scene = m_scene.lock();

    // Update scene rules if simulation is running and not paused
    if (m_isGameRunning && !m_isGamePaused)
    {
        m_editor.GetEngineContents().io->UpdateInputs(); // Ensure we have the latest input state for this frame before updating the scene.
        auto l_physics = RE::Physics::Manager::Get().lock();
        l_physics->Step(l_dt);

        l_scene->Update(l_dt);
    }

    // Render the infinite grid first (before ImGui)
    if (!m_isGameRunning && m_sceneSettings.showGrid)
    {
        glBindVertexArray(m_dummyGridVAO);

        // Disable depth testing for grid
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        m_gridShader->Use();
        m_gridShader->SetUniform("u_Projection", m_camera->GetProjection());
        m_gridShader->SetUniform("u_View", m_camera->GetView());
        m_gridShader->SetUniform("u_cameraPos", m_camera->GetPosition());

        float l_cameraWidth = m_camera->GetCameraWidth();
        Vector2 l_viewportSize = m_window->GetScreenSize();
        float l_aspectRatio = l_viewportSize.x / l_viewportSize.y;
        
        // Calculate the height based on the width and viewport aspect ratio to prevent stretching
        float l_orthoHeight = l_cameraWidth / l_aspectRatio;

        m_gridShader->SetUniform("u_cameraSize", Vector2(l_cameraWidth, l_orthoHeight));

        glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);

        // Re-enable depth testing
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        glBindVertexArray(0);
        glUseProgram(0);
    }
    
    if (l_scene)
    {
        // Begin screen-space UI rendering (sets ortho projection, reads mouse).
        Vector2 l_screenSize = m_window->GetScreenSize();
        m_editor.GetUIRenderer().BeginFrame(l_screenSize.x, l_screenSize.y);

        l_scene->Draw();
    }

    // Draw gizmo over selected entity (editor mode only).
    if (!m_isGameRunning && m_sceneSettings.showGizmos && m_selectedEntityId >= 0)
    {
        RE::Core::EntityRegistry& l_reg = l_scene->GetRegistry();

        // Validate primary selected entity still exists
        if (!l_reg.GetEntityTable().raw_get<sol::object>(m_selectedEntityId).valid())
        {
            m_selectedEntityId = -1;
            m_selectedEntities.clear();
        }
        else
        {
            // Draw selection highlight on all selected entities
            for (int l_id : m_selectedEntities)
            {
                if (l_id == m_selectedEntityId) continue; // primary gets gizmo instead
                if (!l_reg.GetEntityTable().raw_get<sol::object>(l_id).valid()) continue;
                Vector2 l_pos = l_reg.GetEntityAttributes(l_id)
                                    .traverse_raw_get<Vector2>("Transform", "Position");
                Vector2 l_scale = l_reg.GetEntityAttributes(l_id)
                                    .traverse_raw_get<Vector2>("Transform", "Scale");
                Vector2 l_sp = m_camera->WorldToScreenPoint(l_pos);
                Vector2 l_se = m_camera->WorldToScreenPoint(l_pos + l_scale);
                float l_sh = m_window->GetScreenSize().y;
                ImVec2 l_center(l_sp.x, l_sh - l_sp.y);
                ImVec2 l_half(std::abs(l_se.x - l_sp.x), std::abs(l_se.y - l_sp.y));
                ImGui::GetForegroundDrawList()->AddRect(
                    ImVec2(l_center.x - l_half.x, l_center.y - l_half.y),
                    ImVec2(l_center.x + l_half.x, l_center.y + l_half.y),
                    IM_COL32(0, 200, 255, 180), 0.0f, 0, 2.0f);
            }

            // Draw gizmo on primary selected entity
            Vector2 l_entityPos = l_reg.GetEntityAttributes(m_selectedEntityId)
                                    .traverse_raw_get<Vector2>("Transform", "Position");
            Vector2 l_entityScale = l_reg.GetEntityAttributes(m_selectedEntityId)
                                        .traverse_raw_get<Vector2>("Transform", "Scale");
            Vector2 l_screenPos = m_camera->WorldToScreenPoint(l_entityPos);
            Vector2 l_screenEdges = m_camera->WorldToScreenPoint(l_entityPos + l_entityScale);
            
            float l_screenH = m_window->GetScreenSize().y;
            ImVec2 l_screenHE(std::abs(l_screenEdges.x - l_screenPos.x),
                            std::abs(l_screenEdges.y - l_screenPos.y));
            
            Gizmo::Draw(m_sceneSettings.gizmoMode,
                        ImVec2(l_screenPos.x, l_screenH - l_screenPos.y),
                        l_screenHE,
                        m_activeGizmoAxis);
        }
    }

    // Draw drag-select rectangle
    if (m_isDraggingRect)
    {
        ImVec2 l_min(std::min(m_dragRectStart.x, m_dragRectEnd.x),
                     std::min(m_dragRectStart.y, m_dragRectEnd.y));
        ImVec2 l_max(std::max(m_dragRectStart.x, m_dragRectEnd.x),
                     std::max(m_dragRectStart.y, m_dragRectEnd.y));
        ImGui::GetForegroundDrawList()->AddRect(l_min, l_max, IM_COL32(0, 200, 255, 200), 0.0f, 0, 1.5f);
        ImGui::GetForegroundDrawList()->AddRectFilled(l_min, l_max, IM_COL32(0, 200, 255, 30));
    }

    // Viewport drop target: accept texture/model drops onto the scene
    if (!m_isGameRunning && m_selectedEntityId >= 0)
    {
        ImGuiIO& l_dropIO = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(l_dropIO.DisplaySize);
        ImGui::SetNextWindowBgAlpha(0.0f);
        if (ImGui::Begin("##ViewportDropTarget", nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoDocking))
        {
            if (ImGui::BeginDragDropTarget())
            {
                auto l_applyPath = [&](const char* _payloadType, const char* _catName, const char* _fieldName)
                {
                    const ImGuiPayload* l_payload = ImGui::AcceptDragDropPayload(_payloadType);
                    if (l_payload)
                    {
                        std::string l_path((const char*)l_payload->Data, l_payload->DataSize - 1);
                        auto l_scn = m_scene.lock();
                        if (l_scn)
                        {
                            sol::table l_attrs = l_scn->GetRegistry().GetEntityAttributes(m_selectedEntityId);
                            sol::object l_catObj = l_attrs.raw_get<sol::object>(_catName);
                            if (l_catObj.valid() && l_catObj.get_type() == sol::type::table)
                            {
                                sol::table l_cat = l_catObj.as<sol::table>();
                                l_cat.raw_set(_fieldName, l_path);
                            }
                        }
                    }
                };

                l_applyPath(AssetBrowserPanel::k_DragDropTexture, "Texture", "TexturePath");
                l_applyPath(AssetBrowserPanel::k_DragDropModel, "Model", "ModelPath");
                ImGui::EndDragDropTarget();
            }
        }
        ImGui::End();
    }

	DrawEditorUI();
}

void SceneEditTab::Run()
{
    // Snapshot scene state (including current settings) before simulation begins.
    m_savedSceneState = SceneSerializer::Serialize(m_editor.GetEngineContents(),
                                                   m_sceneSettings);

    // Apply scene gravity to the physics world before Init() runs.
    if (auto l_physics = RE::Physics::Manager::Get().lock())
    {
        l_physics->SetGravity(Vector2(m_sceneSettings.gravityX, m_sceneSettings.gravityY));
    }
    
    auto l_scene = m_scene.lock();
    l_scene->Init();

    m_lastFrameTime = SDL_GetPerformanceCounter();
    m_isGameRunning = true;
}

void SceneEditTab::Pause()
{
    m_isGamePaused = true;
    m_lastFrameTime = SDL_GetPerformanceCounter();
}

void SceneEditTab::Resume()
{
    m_isGamePaused = false;
    m_lastFrameTime = SDL_GetPerformanceCounter();
}

void SceneEditTab::Stop()
{
    m_isGameRunning = false;
    m_isGamePaused  = false;

    if (m_savedSceneState.empty())
    {
        return;
    }

    auto& l_engineContents = m_editor.GetEngineContents();

    l_engineContents.physics->Reset();

    // Restore the scene to its pre-play state; also recover saved settings.
    SceneSerializer::LoadFromString(m_savedSceneState, l_engineContents, &m_sceneSettings);
    m_savedSceneState.clear();

    // Re-add built-in rules (they are excluded from serialisation).
    BuiltinRules::Load(l_engineContents);

    // Re-add any registry rules that the snapshot may not have included.
    ReapplyRegistryToScene();

    // Update local scene reference and refresh panels.
    m_scene = l_engineContents.core->GetScene();
    m_entityPanel.RefreshEntityList();
    m_selectedEntityId = -1;
    m_selectedEntities.clear();
    m_propertiesPanel.Reset();
    m_propertiesPanel.SetShown(false);
}

void SceneEditTab::SelectEntity(int _id)
{
    m_selectedEntities.clear();
    if (_id >= 0)
        m_selectedEntities.push_back(_id);
    m_selectedEntityId = _id;
    m_propertiesPanel.SetShown(_id >= 0);
}

void SceneEditTab::ToggleEntitySelection(int _id)
{
    auto l_it = std::find(m_selectedEntities.begin(), m_selectedEntities.end(), _id);
    if (l_it != m_selectedEntities.end())
    {
        m_selectedEntities.erase(l_it);
    }
    else
    {
        m_selectedEntities.push_back(_id);
    }

    // Update primary selection
    if (m_selectedEntities.empty())
    {
        m_selectedEntityId = -1;
        m_propertiesPanel.SetShown(false);
    }
    else
    {
        m_selectedEntityId = m_selectedEntities.back();
        m_propertiesPanel.SetShown(true);
    }
}

void SceneEditTab::SelectAllEntities()
{
    auto l_scene = m_scene.lock();
    if (!l_scene) return;

    m_selectedEntities = l_scene->GetRegistry().GetAllRegisteredIds();
    if (!m_selectedEntities.empty())
    {
        m_selectedEntityId = m_selectedEntities.front();
        m_propertiesPanel.SetShown(true);
    }
    else
    {
        m_selectedEntityId = -1;
        m_propertiesPanel.SetShown(false);
    }
}

void SceneEditTab::ClearSelection()
{
    m_selectedEntities.clear();
    m_selectedEntityId = -1;
    m_propertiesPanel.SetShown(false);
}

bool SceneEditTab::IsEntitySelected(int _id) const
{
    return std::find(m_selectedEntities.begin(), m_selectedEntities.end(), _id)
           != m_selectedEntities.end();
}

int SceneEditTab::GetSelectedEntity()
{
	return m_selectedEntityId;
}

Panel* SceneEditTab::GetPanelByName(const std::string& _name)
{
    if (m_entityPanel.GetTitle() == _name)
    {
        return &m_entityPanel;
    }
    else if (m_categoryPanel.GetTitle() == _name)
    {
        return &m_categoryPanel;
    }
    else if (m_rulesPanel.GetTitle() == _name)
    {
        return &m_rulesPanel;
    }
    else if (m_cameraPanel.GetTitle() == _name)
    {
        return &m_cameraPanel;
    }
    else if (m_propertiesPanel.GetTitle() == _name)
    {
        return &m_propertiesPanel;
    }
    else if (m_settingsPanel.GetTitle() == _name)
    {
        return &m_settingsPanel;
    }
    else if (m_consolePanel.GetTitle() == _name)
    {
        return &m_consolePanel;
    }
    else if (m_assetBrowserPanel.GetTitle() == _name)
    {
        return &m_assetBrowserPanel;
    }

    return nullptr;
}

void SceneEditTab::RegisterRule(const std::string& _name, const std::string& _filePath)
{
    for (const auto& entry : m_ruleRegistry)
    {
        if (entry.name == _name)
        {
            // Rule already registered
            return;
        }
    }

    m_ruleRegistry.push_back({ _name, _filePath });
}

void SceneEditTab::ClearRuleRegistry()
{
    m_ruleRegistry.clear();
}

void SceneEditTab::RebuildRegistryFromScene()
{
    m_ruleRegistry.clear();

    auto l_scene = m_editor.GetEngineContents().core->GetScene().lock();
    if (!l_scene) return;

    sol::table l_rulesTable = l_scene->GetSceneTable().raw_get<sol::table>("Rules");

    for (auto& l_pair : l_rulesTable.pairs())
    {
        if (l_pair.first.get_type() != sol::type::string)
        {
            continue;
        }

        const std::string l_name = l_pair.first.as<std::string>();
        if (BuiltinRules::IsBuiltin(l_name))
        {
            continue;
        }

        std::shared_ptr<RE::Core::Rule> l_rulePtr =
            l_rulesTable.raw_get<std::shared_ptr<RE::Core::Rule>>(l_name);

        std::string l_path;
        if (l_rulePtr)
        {
            auto l_file = l_rulePtr->GetOriginFile().lock();
            if (l_file) 
            {
                l_path = l_file->GetPath();
            }
        }

        m_ruleRegistry.push_back({ l_name, l_path });
    }
}

void SceneEditTab::ReapplyRegistryToScene()
{
    auto& l_contents = m_editor.GetEngineContents();
    auto l_scene = l_contents.core->GetScene().lock();
    if (!l_scene)
    {
        return;
    }

    sol::table l_rulesTable = l_scene->GetSceneTable().raw_get<sol::table>("Rules");

    for (const auto& entry : m_ruleRegistry)
    {
        // Skip if already present in the scene.
        auto l_existing = l_rulesTable.raw_get<sol::object>(entry.name);
        if (l_existing.valid() && l_existing.get_type() != sol::type::nil)
        {
            continue;
        }

        if (entry.filePath.empty() || !std::filesystem::exists(entry.filePath))
        {
            continue;
        }

        auto l_file = l_contents.resources->Load<RE::Asset::LuaFile>(entry.filePath);
        RE::Core::Rule l_rule = l_contents.core->GetLuaContext()->CreateRule(l_file);
        l_scene->AddRule(l_rule);
    }
}

static const std::vector<int> s_emptyChildren;

void SceneEditTab::SetEntityParent(int _childId, int _parentId)
{
    // Remove from previous parent's children list
    auto l_oldParentIt = m_entityParent.find(_childId);
    if (l_oldParentIt != m_entityParent.end())
    {
        int l_oldParent = l_oldParentIt->second;
        auto& l_siblings = m_entityChildren[l_oldParent];
        l_siblings.erase(std::remove(l_siblings.begin(), l_siblings.end(), _childId), l_siblings.end());
        if (l_siblings.empty())
            m_entityChildren.erase(l_oldParent);
    }

    if (_parentId >= 0)
    {
        m_entityParent[_childId] = _parentId;
        m_entityChildren[_parentId].push_back(_childId);
    }
    else
    {
        m_entityParent.erase(_childId);
    }
}

int SceneEditTab::GetEntityParent(int _entityId) const
{
    auto l_it = m_entityParent.find(_entityId);
    return (l_it != m_entityParent.end()) ? l_it->second : -1;
}

const std::vector<int>& SceneEditTab::GetEntityChildren(int _entityId) const
{
    auto l_it = m_entityChildren.find(_entityId);
    return (l_it != m_entityChildren.end()) ? l_it->second : s_emptyChildren;
}

bool SceneEditTab::IsRootEntity(int _entityId) const
{
    return m_entityParent.find(_entityId) == m_entityParent.end();
}