#include "Editor/EntityPanel.h"
#include "Editor/Editor.h"
#include "Editor/SceneEditTab.h"

#include "imgui/imgui.h"

// String compatible functions for ImGui
#include "imgui/misc/cpp/imgui_stdlib.h"

EntityPanel::EntityPanel(std::weak_ptr<Editor> _editor)
: Panel("Entity List", _editor)
, m_cachedEntities()
, m_registry(_editor.lock()->GetEngineContents().core->GetScene().lock()->GetRegistry())
{
    RefreshEntityList();
}

EntityPanel::~EntityPanel()
{
}

void EntityPanel::RefreshEntityList()
{
    // Update the registry reference to whatever the active scene's registry is.
    auto l_editor = m_editor.lock();
    auto l_scene = l_editor->GetEngineContents().core->GetScene().lock();
    m_registry = l_scene->GetRegistry();

    if(!m_registry.has_value())
        return;
    
    m_cachedEntities.clear();

    sol::table l_entityTable = m_registry.value()
                                         .get()
                                         .GetEntityTable();

    auto l_entityPairs = l_entityTable.pairs();
    for (auto& l_pair : l_entityPairs)
    {
        int l_entityId = l_pair.first.as<int>();
        m_cachedEntities.push_back(l_entityId);
    }
}

void EntityPanel::Draw()
{
    ImGui::SetWindowSize(ImVec2(300, 600));
    auto l_editor = m_editor.lock();
    auto l_sceneEdit = l_editor->GetSceneEdit().lock();
    int l_selectedEntity = l_sceneEdit->GetSelectedEntity();

    // Keep registry reference up-to-date with the current scene.
    if (l_editor)
    {
        auto l_scene = l_editor->GetEngineContents().core->GetScene().lock();
        if (l_scene)
            m_registry = l_scene->GetRegistry();
    }

    if (ImGui::BeginChild("EntityListPanel", ImVec2(0, 0), true))
    {
        // Button row: Add Entity and conditionally Remove Selected
        float buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0f;

        if (ImGui::Button("+ Add Entity", ImVec2(buttonWidth, 0)))
        {
            AddEntity();
        }

        // Only show Remove button when an entity is selected
        if (l_selectedEntity >= 0)
        {
            ImGui::SameLine();
            if (ImGui::Button("- Remove Entity", ImVec2(buttonWidth, 0)))
            {
                RemoveEntity(l_selectedEntity);
            }
        }

        ImGui::Separator();

        // Entity list
        if (ImGui::BeginChild("EntityList", ImVec2(0, 0), true))
        {
            for (int l_entityId : m_cachedEntities)
            {
                bool selected = (l_selectedEntity == l_entityId);
                std::string l_entityName = m_registry.value()
                                                        .get()
                                                        .GetEntityName(l_entityId);

                if (ImGui::Selectable(l_entityName.c_str(), selected))
                {
                    l_sceneEdit->SelectEntity(l_entityId);
                }
            }
            ImGui::EndChild();
        }

        ImGui::EndChild();
    }
}

void EntityPanel::AddEntity()
{
    auto l_editorPtr = m_editor.lock();
    auto l_sceneEdit = l_editorPtr->GetSceneEdit().lock();

    auto l_scene = l_editorPtr->GetEngineContents().core->GetScene().lock();

    int newId = l_scene->AddEntity();
    m_cachedEntities.push_back(newId);
    l_sceneEdit->SelectEntity(newId);
    RE::Log::Message("Entity created with ID: " + std::to_string(newId));
}

void EntityPanel::RemoveEntity(int _id)
{
    auto l_scene = m_editor.lock()->GetEngineContents()
                                  .core->GetScene().lock();
    l_scene->RemoveEntity(_id);
    
    auto l_entityLocation = std::find(m_cachedEntities.begin(), m_cachedEntities.end(), _id);
    if (l_entityLocation != m_cachedEntities.end())
    {
        m_cachedEntities.erase(l_entityLocation);
    }

    RE::Log::Message("Entity removed with ID: " + std::to_string(_id));
}
