#include "Editor/EntityPanel.h"
#include "Editor/Editor.h"
#include "imgui/imgui.h"

EntityPanel::EntityPanel(std::weak_ptr<Editor> _editor)
: Panel(_editor)
, m_selectedEntity(-1)
{
    RefreshEntityList();
}

EntityPanel::~EntityPanel()
{
}

void EntityPanel::RefreshEntityList()
{
    m_cachedEntities.clear();
    auto l_registry = m_editor.lock()->GetScene()->GetRegistry();

    sol::table l_entityTable = l_registry->GetEntityTable();

    auto l_entityPairs = l_entityTable.pairs();
    for (auto& pair : l_entityPairs)
    {
        int entityId = pair.first.as<int>();
        m_cachedEntities.push_back(entityId);
    }
}

void EntityPanel::Draw()
{
    if (!m_showPanel) return;

    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Entities", &m_showPanel))
    {
        // Add button
        if (ImGui::Button("+ Add Entity", ImVec2(-1, 0)))
        {
            AddEntity();
        }

        ImGui::Separator();

        // Entity list
        if (ImGui::BeginChild("EntityList", ImVec2(0, -50), true))
        {
            for (int entityId : m_cachedEntities)
            {
                bool selected = (m_selectedEntity == entityId);
                if (ImGui::Selectable(("Entity##" + std::to_string(entityId)).c_str(), selected))
                {
                    SelectEntity(entityId);
                }
            }
            ImGui::EndChild();
        }

        // Remove button
        if (ImGui::Button("- Remove Selected", ImVec2(-1, 0)))
        {
            if (m_selectedEntity >= 0)
            {
                RemoveEntity(m_selectedEntity);
            }
        }
    }
    ImGui::End();
}

void EntityPanel::AddEntity()
{
    auto l_editorPtr = m_editor.lock();
    auto l_scene = l_editorPtr->GetScene();

    int newId = l_scene->AddEntity();
    m_cachedEntities.push_back(newId);
    SelectEntity(newId);
    RE::Log::Message("Entity created with ID: " + std::to_string(newId));
}

void EntityPanel::RemoveEntity(int _id)
{
    auto l_scene = m_editor.lock()->GetScene();
    l_scene->RemoveEntity(_id);
    
    auto it = std::find(m_cachedEntities.begin(), m_cachedEntities.end(), _id);
    if (it != m_cachedEntities.end())
    {
        m_cachedEntities.erase(it);
    }

    if (m_selectedEntity == _id)
    {
        m_selectedEntity = -1;
    }
    RE::Log::Message("Entity removed with ID: " + std::to_string(_id));
}

void EntityPanel::SelectEntity(int _id)
{
    m_selectedEntity = _id;
    m_editor.lock()->SetSelectedEntityId(_id);
}
