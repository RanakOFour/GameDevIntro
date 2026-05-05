#include "Editor/Panels/Scene/EntityPanel.h"
#include "Editor/Core/Editor.h"
#include "Editor/Tabs/SceneEditTab.h"
#include "Editor/Core/UndoManager.h"

#include "imgui/imgui.h"

#include "sol/sol.hpp"

// String compatible functions for ImGui
#include "imgui/misc/cpp/imgui_stdlib.h"

EntityPanel::EntityPanel(Editor& _editor)
: Panel("Entity List", _editor)
, m_cachedEntities()
, m_registry(_editor.GetEngineContents().core->GetScene().lock()->GetRegistry())
{
    RefreshEntityList();
}

EntityPanel::~EntityPanel()
{
}

void EntityPanel::RefreshEntityList()
{
    auto l_scene = m_editor.GetEngineContents().core->GetScene().lock();
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
    auto l_sceneEdit = &m_editor.GetSceneEdit();
    int l_selectedEntity = l_sceneEdit->GetSelectedEntity();

    // Keep registry reference up-to-date with the current scene.
    auto l_scene = m_editor.GetEngineContents().core->GetScene().lock();
    if (l_scene)
        m_registry = l_scene->GetRegistry();

    if (ImGui::BeginChild("EntityListPanel", ImVec2(0, 0), true))
    {
        // Search filter
        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextWithHint("##EntitySearch", "Search entities...", &m_searchFilter);
        ImGui::Separator();

        // Button row: Add Entity and conditionally Remove Selected
        float buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0f;

        if (ImGui::Button("+ Add Entity", ImVec2(buttonWidth, 0)))
        {
            AddEntity();
        }

        // Only show Remove button when entities are selected
        if (!l_sceneEdit->GetSelectedEntities().empty())
        {
            ImGui::SameLine();
            if (ImGui::Button("- Remove Entity", ImVec2(buttonWidth, 0)))
            {
                std::vector<int> l_toRemove = l_sceneEdit->GetSelectedEntities();
                for (int l_id : l_toRemove)
                {
                    RemoveEntity(l_id);
                }
                l_sceneEdit->ClearSelection();
            }
        }

        ImGui::Separator();

        // Entity list
        if (ImGui::BeginChild("EntityList", ImVec2(0, 0), true))
        {
            // Lowercase search filter for case-insensitive matching
            std::string l_filterLower = m_searchFilter;
            std::transform(l_filterLower.begin(), l_filterLower.end(), l_filterLower.begin(), ::tolower);

            if (!l_filterLower.empty())
            {
                // Flat filtered list
                for (int l_entityId : m_cachedEntities)
                {
                    std::string l_entityName = m_registry.value()
                                                            .get()
                                                            .GetEntityName(l_entityId);

                    std::string l_nameLower = l_entityName;
                    std::transform(l_nameLower.begin(), l_nameLower.end(), l_nameLower.begin(), ::tolower);
                    if (l_nameLower.find(l_filterLower) == std::string::npos)
                        continue;

                    bool selected = l_sceneEdit->IsEntitySelected(l_entityId);
                    if (ImGui::Selectable(l_entityName.c_str(), selected))
                    {
                        if (ImGui::GetIO().KeyShift)
                            l_sceneEdit->ToggleEntitySelection(l_entityId);
                        else
                            l_sceneEdit->SelectEntity(l_entityId);
                    }
                }
            }
            else
            {
                // Hierarchical tree view: only draw root entities, recurse into children
                for (int l_entityId : m_cachedEntities)
                {
                    if (!l_sceneEdit->IsRootEntity(l_entityId))
                        continue;
                    DrawEntityTreeNode(l_sceneEdit, l_entityId);
                }
            }
            ImGui::EndChild();
        }

        ImGui::EndChild();
    }
}

void EntityPanel::DrawEntityTreeNode(SceneEditTab* _sceneEdit, int _entityId)
{
    std::string l_entityName = m_registry.value().get().GetEntityName(_entityId);
    const auto& l_children = _sceneEdit->GetEntityChildren(_entityId);
    bool l_hasChildren = !l_children.empty();
    bool l_selected = _sceneEdit->IsEntitySelected(_entityId);

    ImGuiTreeNodeFlags l_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (l_selected)
        l_flags |= ImGuiTreeNodeFlags_Selected;
    if (!l_hasChildren)
        l_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    ImGui::PushID(_entityId);
    bool l_open = ImGui::TreeNodeEx(l_entityName.c_str(), l_flags);

    // Click handling
    if (ImGui::IsItemClicked(0) && !ImGui::IsItemToggledOpen())
    {
        if (ImGui::GetIO().KeyShift)
            _sceneEdit->ToggleEntitySelection(_entityId);
        else
            _sceneEdit->SelectEntity(_entityId);
    }

    // Drag source for reparenting
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::SetDragDropPayload("ENTITY_REPARENT", &_entityId, sizeof(int));
        ImGui::Text("Move %s", l_entityName.c_str());
        ImGui::EndDragDropSource();
    }

    // Drop target for reparenting
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* l_payload = ImGui::AcceptDragDropPayload("ENTITY_REPARENT"))
        {
            int l_draggedId = *(const int*)l_payload->Data;
            if (l_draggedId != _entityId)
            {
                _sceneEdit->SetEntityParent(l_draggedId, _entityId);
            }
        }
        ImGui::EndDragDropTarget();
    }

    // Context menu for unparenting
    if (ImGui::BeginPopupContextItem())
    {
        if (_sceneEdit->GetEntityParent(_entityId) >= 0)
        {
            if (ImGui::MenuItem("Unparent"))
            {
                _sceneEdit->SetEntityParent(_entityId, -1);
            }
        }
        ImGui::EndPopup();
    }

    if (l_open && l_hasChildren)
    {
        for (int l_childId : l_children)
        {
            DrawEntityTreeNode(_sceneEdit, l_childId);
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
}

// Shared helpers for snapshotting and restoring entity state across undo/redo.
namespace
{
    using FieldEntry = std::pair<std::string, sol::object>;
    using CatEntry   = std::pair<std::string, std::vector<FieldEntry>>;

    // Captures all category memberships and field values of entity _id into _out.
    void SnapshotEntity(RE::Core::Scene& _scene, int _id,
                        std::string& _nameOut,
                        std::vector<CatEntry>& _out)
    {
        _nameOut = _scene.GetRegistry().GetEntityName(_id);
        _out.clear();
        sol::table l_attrs = _scene.GetRegistry().GetEntityAttributes(_id);
        for (auto& l_catPair : l_attrs)
        {
            std::string l_catName = l_catPair.first.as<std::string>();
            sol::table  l_fields  = l_catPair.second.as<sol::table>();
            std::vector<FieldEntry> l_fieldSnap;
            for (auto& l_fp : l_fields)
                l_fieldSnap.emplace_back(l_fp.first.as<std::string>(), l_fp.second);
            _out.emplace_back(l_catName, std::move(l_fieldSnap));
        }
    }

    // Creates a new entity and restores categories + field values from _snapshot.
    // Returns the new entity ID and updates *_currentId.
    int RestoreEntity(RE::Core::Scene& _scene,
                      const std::string& _name,
                      const std::vector<CatEntry>& _snapshot,
                      std::shared_ptr<int> _currentId)
    {
        int l_newId = _scene.AddEntity();
        *_currentId = l_newId;

        for (auto& [l_catName, l_fields] : _snapshot)
        {
            if (l_catName != "Transform")         // Transform already added by AddEntity
                _scene.AddToCategory(l_newId, l_catName);
        }

        sol::table l_attrs = _scene.GetRegistry().GetEntityAttributes(l_newId);
        for (auto& [l_catName, l_fields] : _snapshot)
        {
            sol::object l_catObj = l_attrs.raw_get<sol::object>(l_catName.c_str());
            if (!l_catObj.valid() || l_catObj.get_type() != sol::type::table)
                continue;
            sol::table l_catAttrs = l_catObj.as<sol::table>();
            for (auto& [l_fname, l_fval] : l_fields)
                l_catAttrs[l_fname] = l_fval;
        }

        if (!_name.empty())
            _scene.RenameEntity(l_newId, _name);

        return l_newId;
    }
} // namespace

void EntityPanel::AddEntity()
{
    auto l_sceneEdit = &m_editor.GetSceneEdit();

    auto l_scene = m_editor.GetEngineContents().core->GetScene().lock();

    int newId = l_scene->AddEntity();
    m_cachedEntities.push_back(newId);
    l_sceneEdit->SelectEntity(newId);
    m_editor.FireTutorialEvent("entity_created");
    RE::Log::Message("Entity created with ID: " + std::to_string(newId));

    Editor* l_edRaw = &m_editor;

    // l_currentId tracks which entity ID is currently live so undo/redo chains
    // stay correct even when the recycled ID differs across cycles.
    auto l_currentId    = std::make_shared<int>(newId);
    auto l_snapshot     = std::make_shared<std::vector<CatEntry>>();
    auto l_snapshotName = std::make_shared<std::string>();

    m_editor.GetUndoManager().PushCommand(
        std::make_unique<LambdaCommand>(
            "Create Entity",
            // Redo: recreate entity and restore snapshot
            [l_edRaw, l_currentId, l_snapshot, l_snapshotName]() {
                auto l_sc = l_edRaw->GetEngineContents().core->GetScene().lock();
                int l_redoId = RestoreEntity(*l_sc, *l_snapshotName, *l_snapshot, l_currentId);
                l_edRaw->GetSceneEdit().GetEntityPanel().RefreshEntityList();
                l_edRaw->GetSceneEdit().SelectEntity(l_redoId);
            },
            // Undo: snapshot current state then remove
            [l_edRaw, l_currentId, l_snapshot, l_snapshotName]() {
                auto l_sc = l_edRaw->GetEngineContents().core->GetScene().lock();
                SnapshotEntity(*l_sc, *l_currentId, *l_snapshotName, *l_snapshot);
                l_sc->RemoveEntity(*l_currentId);
                l_edRaw->GetSceneEdit().GetEntityPanel().RefreshEntityList();
                l_edRaw->GetSceneEdit().SelectEntity(-1);
            }
        )
    );
}

void EntityPanel::RemoveEntity(int _id)
{
    auto l_scene = m_editor.GetEngineContents().core->GetScene().lock();
    auto& l_sceneEdit = m_editor.GetSceneEdit();

    // Remove children first (cascade delete)
    std::vector<int> l_children = l_sceneEdit.GetEntityChildren(_id);
    for (int l_childId : l_children)
    {
        RemoveEntity(l_childId);
    }

    // Clean up hierarchy
    l_sceneEdit.SetEntityParent(_id, -1);

    // Snapshot BEFORE removal so undo can restore the entity fully.
    auto l_snapshot     = std::make_shared<std::vector<CatEntry>>();
    auto l_snapshotName = std::make_shared<std::string>();
    SnapshotEntity(*l_scene, _id, *l_snapshotName, *l_snapshot);

    l_scene->RemoveEntity(_id);

    auto l_entityLocation = std::find(m_cachedEntities.begin(), m_cachedEntities.end(), _id);
    if (l_entityLocation != m_cachedEntities.end())
        m_cachedEntities.erase(l_entityLocation);

    RE::Log::Message("Entity removed with ID: " + std::to_string(_id));

    // l_currentId starts as _id; updated to the new ID when undo recreates it.
    Editor* l_edRaw = &m_editor;
    auto l_currentId = std::make_shared<int>(_id);
    l_edRaw->GetUndoManager().PushCommand(
        std::make_unique<LambdaCommand>(
            "Delete Entity",
            // Redo: remove entity again
            [l_edRaw, l_currentId]() {
                auto l_sc = l_edRaw->GetEngineContents().core->GetScene().lock();
                l_sc->RemoveEntity(*l_currentId);
                l_edRaw->GetSceneEdit().GetEntityPanel().RefreshEntityList();
                l_edRaw->GetSceneEdit().SelectEntity(-1);
            },
            // Undo: recreate entity and restore snapshot
            [l_edRaw, l_currentId, l_snapshot, l_snapshotName]() {
                auto l_sc = l_edRaw->GetEngineContents().core->GetScene().lock();
                int l_undoId = RestoreEntity(*l_sc, *l_snapshotName, *l_snapshot, l_currentId);
                l_edRaw->GetSceneEdit().GetEntityPanel().RefreshEntityList();
                l_edRaw->GetSceneEdit().SelectEntity(l_undoId);
            }
        )
    );
}
