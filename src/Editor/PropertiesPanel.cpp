#include "Editor/PropertiesPanel.h"
#include "Editor/Editor.h"
#include "imgui/imgui.h"

PropertiesPanel::PropertiesPanel(std::weak_ptr<Editor> _editor)
: Panel(_editor)
, m_displayedEntityId(-1)
, m_showAddToCategoryMenu(false)
{
}

PropertiesPanel::~PropertiesPanel()
{
}

void PropertiesPanel::Draw()
{
    if (!m_showPanel) return;

    auto l_editor = m_editor.lock();

    int l_selectedEntity = l_editor->GetSelectedEntityId();
    if(l_selectedEntity < 0)
    {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Properties", &m_showPanel))
    {
        ImGui::Text("Entity ID: %d", l_selectedEntity);
        ImGui::Separator();

        DrawEntityProperties(l_selectedEntity);

        if(ImGui::Button("Add to Category", ImVec2(-1, 0)))
        {
            auto l_registry = l_editor->GetScene()->GetRegistry();
            sol::table l_categories = l_registry->GetCategoryTable();
            for (auto& l_pair : l_categories)
            {
                std::string l_name = l_pair.first.as<std::string>();
                if (ImGui::MenuItem(l_name.c_str()))
                {
                    l_registry->AddToCategory(l_selectedEntity, l_pair.second.as<std::bitset<1024>>());
                    m_showAddToCategoryMenu = false;
                }
            }
        }

        if (m_showAddToCategoryMenu)
        {
            // Need to pull categories from Category Panel instead
            auto l_registry = l_editor->GetScene()->GetRegistry();
            sol::table l_categories = l_registry->GetCategoryTable();
            for (auto& l_pair : l_categories)
            {
                std::string l_name = l_pair.first.as<std::string>();
                if (ImGui::MenuItem(l_name.c_str()))
                {
                    l_registry->AddToCategory(l_selectedEntity, l_pair.second.as<std::bitset<1024>>());
                    m_showAddToCategoryMenu = false;
                }
            }
        }
    }
    ImGui::End();
}

void PropertiesPanel::DrawEntityProperties(int _entityId)
{
    auto l_registry = m_editor.lock()->GetScene()->GetRegistry();

    try
    {
        sol::table l_entityData = l_registry->GetEntityAttributes(_entityId);
        
        // Display each category and its attributes
        for (auto& l_pair : l_entityData)
        {
            std::string categoryName = l_pair.first.as<std::string>();
            sol::table attributes = l_pair.second.as<sol::table>();

            if (ImGui::CollapsingHeader(categoryName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Indent();
                DrawCategoryAttributes(categoryName, attributes);
                ImGui::Unindent();
            }
        }
    }
    catch (const std::exception& e)
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: Could not load entity properties");
    }
}

void PropertiesPanel::DrawCategoryAttributes(const std::string& _categoryName, sol::table& _attributes)
{
    float columnWidth = ImGui::GetColumnWidth() - 30;

    std::vector<std::string> l_properties;

    for(auto& l_pair : _attributes)
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

        switch(l_value.get_type())
        {
            case sol::type::number:
                if (l_value.is<int>())
                {
                    int intVal = l_value.as<int>();
                    if (ImGui::InputInt(l_property.c_str(), &intVal))
                    {
                        _attributes[l_property] = intVal;
                    }
                }
                else if (l_value.is<float>())
                {
                    float floatVal = l_value.as<float>();
                    if (ImGui::InputFloat(l_property.c_str(), &floatVal))
                    {
                        _attributes[l_property] = floatVal;
                    }
                }
                break;
            case sol::type::boolean:
                {
                    bool boolVal = l_value.as<bool>();
                    if (ImGui::Checkbox(l_property.c_str(), &boolVal))
                    {
                        _attributes[l_property] = boolVal;
                    }
                }
                break;
            case sol::type::string:
                {
                    // This is terrible, change later
                    // Use ID+Category+AttributeName as key
                    static std::map<std::string, std::string> stringBuffers;
                    std::string stringVal = l_value.as<std::string>();
                    
                    stringBuffers[l_property] = stringVal;
                    if (ImGui::InputText(l_property.c_str(), &stringBuffers[l_property][0], ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        _attributes[l_property] = stringBuffers[l_property];
                    }
                }
                break;
            case sol::type::userdata:
                // Assuming Vector2 is exposed as userdata
                if (l_value.is<Vector2>())
                {
                    Vector2 vec = l_value.as<Vector2>();
                    if (ImGui::InputFloat2(l_property.c_str(), &vec.x))
                    {
                        _attributes[l_property] = vec;
                    }
                }
                else if(l_value.is<Vector3>())
                {
                    Vector3 vec = l_value.as<Vector3>();
                    if (ImGui::InputFloat3(l_property.c_str(), &vec.x))
                    {
                        _attributes[l_property] = vec;
                    }
                }
                break;
            default:
                ImGui::TextDisabled("%s: Unsupported type", l_property.c_str());
        }

        ImGui::PopID();
    }
}
