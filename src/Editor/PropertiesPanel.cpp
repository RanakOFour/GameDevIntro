#include "Editor/PropertiesPanel.h"
#include "Editor/Editor.h"
#include "Editor/SceneEditTab.h"
#include "Editor/AssetBrowserPanel.h"

#include "RanakEngine/RanakEngine.h"

#include "imgui/imgui.h"

#include "imgui/misc/cpp/imgui_stdlib.h"

#define USE_STD_FILESYSTEM 1
#include "imguiFileDialog/ImGuiFileDialog.h"

#include "sol/sol.hpp"

#include <algorithm>
#include <memory>

PropertiesPanel::PropertiesPanel(Editor& _editor)
: Panel("Entity Properties", _editor)
, m_stringValueMap()
, m_entityNameMap()
, m_showAddToCategory(false)
, m_showPathDialog(false)
, m_pendingPathKey()
, m_pendingPathProperty()
, m_pendingPathTable()
, m_setPosition(false)
, m_position(0, 0)
, m_size(400.0f, 500.0f)
, m_registry(_editor.GetEngineContents().core->GetScene().lock()->GetRegistry())
{

}

PropertiesPanel::~PropertiesPanel()
{

}

void PropertiesPanel::DrawEntityProperties(int _id)
{
    sol::table l_entityData = m_registry.value().get().GetEntityAttributes(_id);

    // Display each category and its attributes
    for (auto& l_pair : l_entityData)
    {
        std::string l_categoryName = l_pair.first.as<std::string>();
        sol::table l_attributes = l_pair.second.as<sol::table>();

        if (ImGui::CollapsingHeader(l_categoryName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Indent();
            DrawCategoryAttributes(_id, l_categoryName, l_attributes);
            ImGui::Unindent();
        }
    }
}

void PropertiesPanel::DrawCategoryAttributes(int _entityId, std::string _categoryName, sol::table _attributes)
{
    // Fetch base category fields once so hidden metadata can be checked per property.
    sol::table l_baseFields;
    auto l_category = m_editor.GetEngineContents()
                               .core->GetLuaContext()
                               ->GetCategory(_categoryName).lock();
    if (l_category)
    {
        l_baseFields = l_category->GetBaseData();
    }

    std::vector<std::string> l_properties;

    for (auto& l_pair : _attributes)
    {
        l_properties.push_back(l_pair.first.as<std::string>());
    }

    // Sort property names lexographically, probably a better way to do this than every frame
    std::sort(l_properties.begin(), l_properties.end());

    for (auto& l_property : l_properties)
    {
        // Skip properties marked hidden via Field(default, { hidden = true }) in the category definition.
        if (l_baseFields.valid())
        {
            sol::object l_baseMeta = l_baseFields.raw_get<sol::object>(l_property.c_str());
            if (l_baseMeta.get_type() == sol::type::table)
            {
                sol::table l_metaTable = l_baseMeta.as<sol::table>();
                sol::optional<bool> l_isField = l_metaTable["__isField"];
                if (l_isField.has_value() && *l_isField)
                {
                    sol::optional<sol::table> l_opts = l_metaTable["opts"];
                    if (l_opts.has_value())
                    {
                        sol::optional<bool> l_hidden = (*l_opts)["hidden"];
                        if (l_hidden.has_value() && *l_hidden)
                            continue;
                    }
                }
            }
        }

        // Create a unique ID for this property
        ImGui::PushID((_categoryName + "::" + l_property).c_str());

        // Determine the type and display accordingly
        sol::object l_value = _attributes.raw_get<sol::object>(l_property.c_str());

        switch (l_value.get_type())
        {
        case sol::type::number:
            if (l_value.is<int>())
            {
                int l_val = l_value.as<int>();
                if (ImGui::InputInt(l_property.c_str(), &l_val))
                {
                    _attributes[l_property] = l_val;
                }
            }
            else if (l_value.is<float>())
            {
                float l_val = l_value.as<float>();
                if (ImGui::InputFloat(l_property.c_str(), &l_val, 0.1f, 0.0f, "%.3f"))
                {
                    _attributes[l_property] = l_val;
                }
            }
            break;
        case sol::type::boolean:
        {
            bool l_val = l_value.as<bool>();
            if (ImGui::Checkbox(l_property.c_str(), &l_val))
            {
                _attributes[l_property] = l_val;
            }
        }
        break;
        case sol::type::string:
        {
            // Use ID+Category+AttributeName as key, should be specific enough
            std::string l_key = std::to_string(_entityId) + _categoryName + l_property;

            if (m_stringValueMap.find(l_key) == m_stringValueMap.end())
            {
                m_stringValueMap[l_key] = l_value.as<std::string>();
            }

            // Properties with "Path" in their name get a browse button that opens a file dialog.
            bool l_isPath = l_property.find("Path") != std::string::npos ||
                            l_property.find("path") != std::string::npos;

            if (l_isPath)
            {
                // Reserve space for the "Select File" button to the right.
                float l_buttonWidth = ImGui::CalcTextSize("Select File").x + ImGui::GetStyle().FramePadding.x * 2.0f;
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - l_buttonWidth - ImGui::GetStyle().ItemSpacing.x);
                if (ImGui::Button("Select File", ImVec2(l_buttonWidth, 0)))
                {
                    m_pendingPathKey      = l_key;
                    m_pendingPathProperty = l_property;
                    m_pendingPathTable    = _attributes;
                    m_showPathDialog      = true;
                }

                ImGui::SameLine();
            }
            
            if (ImGui::InputText(l_property.c_str(), &m_stringValueMap[l_key], ImGuiInputTextFlags_EnterReturnsTrue))
            {
                _attributes[l_property] = m_stringValueMap[l_key];
            }

            // Accept drag-drop of textures and models onto Path properties
            if (l_isPath && ImGui::BeginDragDropTarget())
            {
                const ImGuiPayload* l_payload = ImGui::AcceptDragDropPayload(AssetBrowserPanel::k_DragDropTexture);
                if (!l_payload)
                    l_payload = ImGui::AcceptDragDropPayload(AssetBrowserPanel::k_DragDropModel);

                if (l_payload)
                {
                    std::string l_droppedPath((const char*)l_payload->Data, l_payload->DataSize - 1);
                    m_stringValueMap[l_key] = l_droppedPath;
                    _attributes[l_property] = l_droppedPath;
                }
                ImGui::EndDragDropTarget();
            }
        }
        break;
        case sol::type::userdata:
            // Assuming Vector2 is exposed as userdata
            if (l_value.is<Vector2>())
            {
                Vector2 l_val = l_value.as<Vector2>();
                if (ImGui::InputFloat2(l_property.c_str(), &l_val.x, "%.3f"))
                {
                    _attributes[l_property] = l_val;
                }
            }
            else if (l_value.is<Vector3>())
            {
                Vector3 l_val = l_value.as<Vector3>();
                if (ImGui::InputFloat3(l_property.c_str(), &l_val.x, "%.3f"))
                {
                    _attributes[l_property] = l_val;
                }
            }
            else if (l_value.is<Vector4>())
            {
                Vector4 l_val = l_value.as<Vector4>();
                if (ImGui::InputFloat4(l_property.c_str(), &l_val.x, "%.3f"))
                {
                    _attributes[l_property] = l_val;
                }
            }
            break;
        default:
            ImGui::TextDisabled("%s: Unsupported type", l_property.c_str());
        }

        ImGui::PopID();
    }
}

void PropertiesPanel::Draw()
{
    int l_selectedEntity = m_editor.GetSceneEdit().GetSelectedEntity();
    const auto& l_selectedEntities = m_editor.GetSceneEdit().GetSelectedEntities();

    auto l_scene = m_editor.GetEngineContents().core->GetScene().lock();
    if (l_scene)
    {
        m_registry = l_scene->GetRegistry();
    }

    // Only set position initially to prevent locking the panel
    if(m_setPosition)
    {
        ImGui::SetWindowPos(m_position);
        m_setPosition = false;
    }

    ImGui::SetWindowSize(m_size);
    if (l_selectedEntities.size() > 1)
    {
        ImGui::Text("%d entities selected", (int)l_selectedEntities.size());
        ImGui::Separator();

        // Show properties of the primary selected entity
        if (l_selectedEntity > -1)
        {
            DrawEntityProperties(l_selectedEntity);
        }
    }
    else if (l_selectedEntity > -1)
    {
        ImGui::Text("Entity ID: %d", l_selectedEntity);
        ImGui::Separator();

        if (m_entityNameMap.find(l_selectedEntity) == m_entityNameMap.end())
        {
            m_entityNameMap[l_selectedEntity] = m_registry.value().get().GetEntityName(l_selectedEntity);
        }

        if (ImGui::InputText("Name", &m_entityNameMap[l_selectedEntity], ImGuiInputTextFlags_EnterReturnsTrue))
        {
            sol::table l_entityTable = m_registry.value().get().GetEntityTable().raw_get<sol::table>(l_selectedEntity);
            l_entityTable["name"] = m_entityNameMap[l_selectedEntity];
        }

        ImGui::Separator();

        DrawEntityProperties(l_selectedEntity);

        if (ImGui::Button("Add to Category", ImVec2(-1, 0)))
        {
            m_showAddToCategory = !m_showAddToCategory;
        }

        if (m_showAddToCategory)
        {
            ImGui::Separator();

            // I love cache!!! Bleh :3
            std::stringstream l_categories(m_editor.GetEngineContents()
                .core->GetLuaContext()
                ->GetCategoryNames());
            std::string l_categoryName;

            sol::table l_entityData = m_registry.value().get().GetEntityAttributes(l_selectedEntity);

            while (std::getline(l_categories, l_categoryName, ';'))
            {
                bool l_showCategory = true;

                // Rule out categories that are already on the entity
                for (auto& l_pair : l_entityData)
                {
                    std::string l_entityCatName = l_pair.first.as<std::string>();
                    if (l_entityCatName == l_categoryName)
                    {
                        l_showCategory = false;
                    }
                }

                //Only show unknown categories
                if (l_showCategory)
                {
                    if (ImGui::MenuItem(l_categoryName.c_str()))
                    {
                        // Peak cache optimisation. A s_ptr<LuaContext> would probably be best
                        auto l_category = m_editor.GetEngineContents()
                            .core->GetLuaContext()
                            ->GetCategory(l_categoryName).lock();

                        m_registry.value().get().AddToCategory(l_selectedEntity, l_category->GetSignature());
                        m_showAddToCategory = false;
                    }
                }
            }
        }
    }
    else
    {
        ImGui::Text("No entity selected");
    }

    DrawPathDialog();
}

void PropertiesPanel::DrawPathDialog()
{
    if (!m_showPathDialog)
        return;

    // Derive a sensible file filter from the property name.
    std::string l_prop = m_pendingPathProperty;
    std::transform(l_prop.begin(), l_prop.end(), l_prop.begin(), ::tolower);

    const char* l_filter = ".*";
    if      (l_prop.find("model")   != std::string::npos) l_filter = ".obj";
    else if (l_prop.find("texture") != std::string::npos) l_filter = ".png,.jpg,.jpeg,.bmp,.tga";
    else if (l_prop.find("shader")  != std::string::npos) l_filter = ".vs,.fs,.vert,.frag";

    IGFD::FileDialogConfig config;
    config.path = ".";
    ImGuiFileDialog::Instance()->OpenDialog("PropPathFileDlg", "Choose File", l_filter, config);

    if (ImGuiFileDialog::Instance()->Display("PropPathFileDlg"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string l_chosen = ImGuiFileDialog::Instance()->GetFilePathName();
            m_stringValueMap[m_pendingPathKey] = l_chosen;
            if (m_pendingPathTable.valid())
            {
                m_pendingPathTable[m_pendingPathProperty] = l_chosen;
            }
        }

        ImGuiFileDialog::Instance()->Close();
        m_showPathDialog = false;
    }
}

void PropertiesPanel::SetPosition(ImVec2 _pos)
{
    m_setPosition = true;
    m_position = _pos;
}

ImVec2 PropertiesPanel::GetSize()
{
    return m_size;
}

void PropertiesPanel::Reset()
{
    m_stringValueMap.clear();
    m_entityNameMap.clear();
    m_showAddToCategory = false;
}