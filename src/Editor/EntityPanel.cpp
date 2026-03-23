#include "Editor/EntityPanel.h"
#include "Editor/Editor.h"

#include "imgui/imgui.h"

// String compatible functions for ImGui
#include "imgui/misc/cpp/imgui_stdlib.h"

EntityPanel::EntityPanel(std::weak_ptr<Editor> _editor)
: Panel(_editor)
, m_selectedEntity(-1)
, m_showAddToCategoryMenu(false)
, m_stringValueMap()
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
    for (auto& l_pair : l_entityPairs)
    {
        int l_entityId = l_pair.first.as<int>();
        m_cachedEntities.push_back(l_entityId);
    }
}

void EntityPanel::Draw()
{
    if (!m_showPanel) return;

    ImGui::SetNextWindowSize(ImVec2(300, 600));
    if (ImGui::Begin("Entities", &m_showPanel))
    {
        auto l_editor = m_editor.lock();
        auto l_registry = l_editor->GetScene()->GetRegistry();

        if (ImGui::BeginChild("EntityListPanel", ImVec2(0, 0), true))
        {
            // Button row: Add Entity and conditionally Remove Selected
            float buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0f;

            if (ImGui::Button("+ Add Entity", ImVec2(buttonWidth, 0)))
            {
                AddEntity();
            }

            // Only show Remove button when an entity is selected
            if (m_selectedEntity >= 0)
            {
                ImGui::SameLine();
                if (ImGui::Button("- Remove Entity", ImVec2(buttonWidth, 0)))
                {
                    RemoveEntity(m_selectedEntity);
                }
            }

            ImGui::Separator();

            // Entity list
            if (ImGui::BeginChild("EntityList", ImVec2(0, 0), true))
            {
                for (int l_entityId : m_cachedEntities)
                {
                    bool selected = (m_selectedEntity == l_entityId);
                    std::string l_entityName = l_registry->GetEntityName(l_entityId);
                    if (ImGui::Selectable(l_entityName.c_str(), selected))
                    {
                        SelectEntity(l_entityId);
                    }
                }
                ImGui::EndChild();
            }

            ImGui::EndChild();
        }

        ImGui::End();
    }
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
    
    auto l_entityLocation = std::find(m_cachedEntities.begin(), m_cachedEntities.end(), _id);
    if (l_entityLocation != m_cachedEntities.end())
    {
        m_cachedEntities.erase(l_entityLocation);
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
