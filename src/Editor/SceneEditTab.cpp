#include "Editor/SceneEditTab.h"

SceneEditTab::SceneEditTab(std::weak_ptr<Editor> _editor)
: m_editor(_editor)
, m_entityPanel(m_editor)
, m_propertiesPanel(m_editor)
, m_categoryPanel(m_editor)
, m_rulesPanel(m_editor)
, m_selectedEntityId(-1)
, m_isGameRunning(false)
{
	auto l_editor = _editor.lock();
	auto l_engineContents = l_editor->GetEngineContents();

	m_scene = l_engineContents.core->GetScene().lock();
	m_camera = l_engineContents.core->GetCamera().lock();
	m_window = l_engineContents.io->GetWindow().lock();

	m_gridShader = l_engineContents.resources->Load<RE::Asset::Shader>("./resources/Shaders/infinite_grid/frag.fs;./resources/Shaders/infinite_grid/vert.vs").lock();
    glGenVertexArrays(1, &m_dummyGridVAO);
    
    auto l_renderRuleFile = l_engineContents.resources->Load<RE::Asset::LuaFile>("./resources/Rules/EditorRender.lua");
    RE::Core::Rule l_renderRule = l_engineContents.core->GetLuaContext()->CreateRule(l_renderRuleFile);

    m_scene->AddRule(l_renderRule);
}

SceneEditTab::~SceneEditTab()
{
	glDeleteVertexArrays(1, &m_dummyGridVAO);

	m_camera.reset();
    m_scene.reset();
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

    m_entityPanel.Draw();
    m_categoryPanel.Draw();
    m_rulesPanel.Draw();
    m_cameraPanel.Draw();
    m_propertiesPanel.Draw();

    if (l_tutLocked)
        ImGui::EndDisabled();
}

void SceneEditTab::DrawContextMenu()
{
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
            sol::table l_entityTransform = m_scene->GetRegistry()
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
                m_scene->RemoveEntity(m_selectedEntityId);
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
	// Draw grid

    // Render the infinite grid first (before ImGui)
    if (m_gridShader)
    //if(false)
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

    // Draw scene with EditorRenderer
    m_scene->Draw();

	DrawEditorUI();
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