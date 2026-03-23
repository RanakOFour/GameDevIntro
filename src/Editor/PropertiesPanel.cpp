#include "Editor/PropertiesPanel.h"
#include "RanakEngine/RanakEngine.h"

#include "imgui/imgui.h"

#include "imgui/misc/cpp/imgui_stdlib.h"

#include "sol/sol.hpp"

PropertiesPanel::PropertiesPanel(std::weak_ptr<Editor> _editor)
: Panel(_editor)
, m_stringValueMap()
, m_entityNameMap()
, m_showCategoryMenu(false)
, m_selectedEntityId(-1)
, m_position(0, 0)
, m_size(400.0f, 500.0f)
{

}

PropertiesPanel::~PropertiesPanel()
{

}

void PropertiesPanel::DrawEntityProperties(int _id)
{
    auto l_registry = m_editor.lock()->GetScene()->GetRegistry();

    sol::table l_entityData = l_registry->GetEntityAttributes(_id);

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

void PropertiesPanel::DrawCategoryAttributes(const int& _entityId, const std::string& _categoryName, sol::table& _attributes)
{
    float columnWidth = ImGui::GetColumnWidth() - 30;

    std::vector<std::string> l_properties;

    for (auto& l_pair : _attributes)
    {
        l_properties.push_back(l_pair.first.as<std::string>());
    }

    // Sort property names lexographically, probably a better way to do this than every frame
    std::sort(l_properties.begin(), l_properties.end());

    for (auto& l_property : l_properties)
    {
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

            if (ImGui::InputText(l_property.c_str(), &m_stringValueMap[l_key], ImGuiInputTextFlags_EnterReturnsTrue))
            {
                _attributes[l_property] = m_stringValueMap[l_key];
            }
        }
        break;
        case sol::type::userdata:
            // Assuming Vector2 is exposed as userdata
            if (l_value.is<Vector2>())
            {
                Vector2 l_val = l_value.as<Vector2>();
                if (ImGui::DragFloat2(l_property.c_str(), &l_val.x, 0.1f, -std::numeric_limits<float>().infinity(), std::numeric_limits<float>().infinity(), "%.3f"))
                {
                    _attributes[l_property] = l_val;
                }
            }
            else if (l_value.is<Vector3>())
            {
                Vector3 l_val = l_value.as<Vector3>();
                if (ImGui::DragFloat3(l_property.c_str(), &l_val.x, 0.1f, -std::numeric_limits<float>().infinity(), std::numeric_limits<float>().infinity(), "%.3f"))
                {
                    _attributes[l_property] = l_val;
                }
            }
            else if (l_value.is<Vector4>())
            {
                Vector4 l_val = l_value.as<Vector4>();
                if (ImGui::DragFloat4(l_property.c_str(), &l_val.x, 0.1f, -std::numeric_limits<float>().infinity(), std::numeric_limits<float>().infinity(), "%.3f"))
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
    if (!m_showPanel || m_selectedEntityId == -1)
        return;

    auto l_editor = m_editor.lock();
    auto l_registry = l_editor->GetScene()->GetRegistry();

    ImGui::SetWindowPos(m_position);
    ImGui::SetWindowSize(m_size);
    if (ImGui::Begin("Entity Properties", &m_showPanel))
    {
        if (m_selectedEntityId > -1)
        {
            ImGui::Text("Entity ID: %d", m_selectedEntityId);
            ImGui::Separator();

            if (m_entityNameMap.find(m_selectedEntityId) == m_entityNameMap.end())
            {
                m_entityNameMap[m_selectedEntityId] = l_registry->GetEntityName(m_selectedEntityId);
            }

            if (ImGui::InputText("Name", &m_entityNameMap[m_selectedEntityId], ImGuiInputTextFlags_EnterReturnsTrue))
            {
                sol::table l_entityTable = l_registry->GetEntityTable().raw_get<sol::table>(m_selectedEntityId);
                l_entityTable["name"] = m_entityNameMap[m_selectedEntityId];
            }

            ImGui::Separator();

            DrawEntityProperties(m_selectedEntityId);

            if (ImGui::Button("Add to Category", ImVec2(-1, 0)))
            {
                m_showCategoryMenu = !m_showCategoryMenu;
            }

            if (m_showCategoryMenu)
            {
                ImGui::Separator();

                // I love cache!!! Bleh :3
                std::stringstream l_categories(l_editor->GetEngineContents()
                    .core->GetLuaContext()
                    ->GetCategoryNames());
                std::string l_categoryName;

                sol::table l_entityData = l_registry->GetEntityAttributes(m_selectedEntityId);

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
                            auto l_category = l_editor->GetEngineContents()
                                .core->GetLuaContext()
                                ->GetCategory(l_categoryName).lock();

                            l_registry->AddToCategory(m_selectedEntityId, l_category->GetSignature());
                            m_showCategoryMenu = false;
                        }
                    }
                }
            }
        }
        else
        {
            ImGui::Text("No entity selected");
        }


        ImGui::End();
    }
}

void PropertiesPanel::SetPosition(ImVec2 _pos)
{
    m_position = _pos;
}

void PropertiesPanel::SetEntity(int _id)
{
    m_selectedEntityId = _id;
    m_showPanel = true;
}

int PropertiesPanel::GetEntity()
{
    return m_selectedEntityId;
}