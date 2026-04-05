#include "Editor/SceneEditTab.h"
#include "Editor/SceneSerializer.h"

#include "RanakEngine/Physics/PhysicsManager.h"

#include "SDL3/SDL.h"

SceneEditTab::SceneEditTab(std::weak_ptr<Editor> _editor)
: m_editor(_editor)
, m_entityPanel(m_editor)
, m_propertiesPanel(m_editor)
, m_categoryPanel(m_editor)
, m_rulesPanel(m_editor)
, m_selectedEntityId(-1)
, m_isGameRunning(false)
, m_lastFrameTime(SDL_GetPerformanceCounter())
{
	auto l_editor = _editor.lock();
	auto l_engineContents = l_editor->GetEngineContents();

	m_scene = l_engineContents.core->GetScene();
	m_camera = l_engineContents.core->GetCamera().lock();
	m_window = l_engineContents.io->GetWindow().lock();

	m_gridShader = l_engineContents.resources->Load<RE::Asset::Shader>("./resources/Shaders/infinite_grid/frag.fs;./resources/Shaders/infinite_grid/vert.vs").lock();
    glGenVertexArrays(1, &m_dummyGridVAO);
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
    TutorialPanel& l_tutorial = m_editor.lock()->GetTutorialPanel();
    const bool l_tutLocked = l_tutorial.IsActive() && !l_tutorial.IsStepInteractive();

    if (!l_tutLocked)
        DrawContextMenu();

    // If the current tutorial step highlights a specific panel, ensure it is
    // visible before Draw() is called so FindWindowByName can locate it.
    const std::string& l_highlightKey = l_tutorial.GetCurrentHighlightKey();
    if (l_highlightKey == "Categories")  m_categoryPanel.SetShown(true);
    else if (l_highlightKey == "Rules")  m_rulesPanel.SetShown(true);

    if (l_tutLocked)
        ImGui::BeginDisabled();

    m_entityPanel.DrawAsWindow();
    m_categoryPanel.DrawAsWindow();
    m_rulesPanel.DrawAsWindow();
    m_cameraPanel.DrawAsWindow();
    m_propertiesPanel.DrawAsWindow();

    if (l_tutLocked)
        ImGui::EndDisabled();
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
			Vector2 l_mousePos = m_editor.lock()->GetEngineContents().io->GetMouseInfo().position;
            Vector3 l_mousePosWorld = m_editor.lock()->GetEngineContents().core->ScreenToWorldPoint(l_mousePos);
            Vector2 l_entityPos(l_mousePosWorld.x, l_mousePosWorld.y);

            m_entityPanel.AddEntity();

            // Awful fucking sentence
            sol::table l_entityTransform = l_scene->GetRegistry()
                                            .GetEntityAttributes(m_selectedEntityId)
                                            .raw_get<sol::table>("Transform");

            l_entityTransform.raw_set("Position", l_entityPos);

            m_propertiesPanel.SetShown(true);
            ImVec2 l_panelSize = m_propertiesPanel.GetSize();

            m_propertiesPanel.SetPosition(ImVec2(l_mousePos.x + l_panelSize.x * 0.25f, l_mousePos.y - l_panelSize.y * 0.25f));
        }

        if(m_selectedEntityId > -1)
        {
            if(ImGui::Button("Delete Entity", l_buttonSize))
            {
                l_scene->RemoveEntity(m_selectedEntityId);
                m_selectedEntityId = -1;

                m_propertiesPanel.SetShown(false);
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
    float l_btnWidth = 80.0f;
    ImGui::SetNextWindowPos(ImVec2((l_io.DisplaySize.x - l_btnWidth) * 0.5f, 24.0f));
    ImGui::SetNextWindowSize(ImVec2(l_btnWidth, 0.0f));
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
            if (ImGui::Button("Stop", ImVec2(l_btnWidth - 16.0f, 0.0f)))
            {
                Stop();
            }
        }
        else
        {
            if (ImGui::Button("Play", ImVec2(l_btnWidth - 16.0f, 0.0f)))
            {
                Run();
            }
        }
    }
    ImGui::End();

    auto l_scene = m_scene.lock();

    // Update scene rules if simulation is running
    if (m_isGameRunning)
    {
        auto l_physics = RE::Physics::Manager::Get().lock();
        l_physics->Step(l_dt);

        l_scene->Update(l_dt);
    }

    // Render the infinite grid first (before ImGui)
    if (!m_isGameRunning)
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
        l_scene->Draw();
    }

	DrawEditorUI();
}

void SceneEditTab::Run()
{
    // Snapshot scene state before simulation begins so we can restore it on Stop.
    m_savedSceneState = SceneSerializer::Serialize(m_editor.lock()->GetEngineContents());

    auto l_scene = m_scene.lock();
    l_scene->Init();

    m_lastFrameTime = SDL_GetPerformanceCounter();
    m_isGameRunning = true;
}

void SceneEditTab::Stop()
{
    m_isGameRunning = false;

    if (m_savedSceneState.empty())
    {
        return;
    }

    auto& l_engineContents = m_editor.lock()->GetEngineContents();

    l_engineContents.physics->Reset();

    // Restore the scene to its pre-play state.
    SceneSerializer::LoadFromString(m_savedSceneState, l_engineContents);
    m_savedSceneState.clear();

    // Re-add the EditorRender rule (it is skipped during serialization).
    auto l_renderRuleFile = l_engineContents.resources->Load<RE::Asset::LuaFile>("./resources/Rules/EditorRender.lua");
    RE::Core::Rule l_renderRule = l_engineContents.core->GetLuaContext()->CreateRule(l_renderRuleFile);
    l_engineContents.core->GetScene().lock()->AddRule(l_renderRule);

    // Update local scene reference and refresh panels.
    m_scene = l_engineContents.core->GetScene();
    m_entityPanel.RefreshEntityList();
    m_selectedEntityId = -1;
    m_propertiesPanel.Reset();
    m_propertiesPanel.SetShown(false);
}

void SceneEditTab::SelectEntity(int _id)
{
    m_selectedEntityId = _id;
    m_propertiesPanel.SetShown(true);
}

int SceneEditTab::GetSelectedEntity()
{
	return m_selectedEntityId;
}