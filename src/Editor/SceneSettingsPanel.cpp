#include "Editor/SceneSettingsPanel.h"
#include "Editor/Editor.h"

#include "RanakEngine/Physics/PhysicsManager.h"
#include "RanakEngine/Math.h"

#include "imgui/imgui.h"

SceneSettingsPanel::SceneSettingsPanel(Editor& _editor, SceneSettings* _settings)
: Panel("Scene Settings", _editor)
, m_settings(_settings)
{
    m_showPanel = false;
}

void SceneSettingsPanel::Draw()
{
    if (!m_settings)
        return;

    ImGui::SeparatorText("Physics");

    float l_gravity[2] = { m_settings->gravityX, m_settings->gravityY };
    if (ImGui::DragFloat2("Gravity", l_gravity, 0.1f, -200.0f, 200.0f, "%.2f"))
    {
        m_settings->gravityX = l_gravity[0];
        m_settings->gravityY = l_gravity[1];

        // Apply immediately so a running simulation picks it up without restart.
        if (auto l_physics = RE::Physics::Manager::Get().lock())
            l_physics->SetGravity(Vector2(m_settings->gravityX, m_settings->gravityY));
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Rendering");

    float l_color[4] = {
        m_settings->clearColorR,
        m_settings->clearColorG,
        m_settings->clearColorB,
        m_settings->clearColorA
    };
    if (ImGui::ColorEdit4("Background", l_color))
    {
        m_settings->clearColorR = l_color[0];
        m_settings->clearColorG = l_color[1];
        m_settings->clearColorB = l_color[2];
        m_settings->clearColorA = l_color[3];
        // Clear colour is read from m_settings every frame in Editor::Draw() — no
        // extra call needed.
    }
}
