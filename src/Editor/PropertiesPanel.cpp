#include "Editor/PropertiesPanel.h"
#include "Editor/Editor.h"
#include "imgui/imgui.h"

PropertiesPanel::PropertiesPanel(std::weak_ptr<Editor> _editor)
: Panel(_editor)
, m_displayedEntityId(-1)
{
}

PropertiesPanel::~PropertiesPanel()
{
}

void PropertiesPanel::Draw()
{
    if (!m_showPanel) return;

    ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Properties##panel", &m_showPanel))
    {
        auto editor = m_editor.lock();
        if (editor)
        {
            int selectedEntity = editor->GetSelectedEntityId();
            
            if (selectedEntity >= 0)
            {
                ImGui::Text("Entity ID: %d", selectedEntity);
                ImGui::Separator();

                static bool l_showAddToCategoryMenu = false;
                if(ImGui::Button("Add to Category", ImVec2(-1, 0)))
                {
                    l_showAddToCategoryMenu = true;
                }

                if (l_showAddToCategoryMenu)
                {
                    auto registry = editor->GetScene()->GetRegistry();
                    sol::table categories = registry->GetCategoryTable();
                    for (auto& pair : categories)
                    {
                        std::string categoryName = pair.first.as<std::string>();
                        if (ImGui::MenuItem(categoryName.c_str()))
                        {
                            registry->AddToCategory(selectedEntity, pair.second.as<std::bitset<1024>>());
                            l_showAddToCategoryMenu = false;
                        }
                    }
                }

                DrawEntityProperties(selectedEntity);
            }
            else
            {
                ImGui::TextDisabled("No entity selected");
                ImGui::TextDisabled("Select an entity from the Entities panel to view/edit its properties");
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

    for (auto& pair : _attributes)
    {
        std::string key = pair.first.as<std::string>();
        
        // Create a unique ID for this property
        ImGui::PushID((_categoryName + "::" + key).c_str());

        // Determine the type and display accordingly
        sol::object value = pair.second;

        switch(value.get_type())
        {
            case sol::type::number:
                if (value.is<int>())
                {
                    int intVal = value.as<int>();
                    if (ImGui::InputInt(key.c_str(), &intVal))
                    {
                        _attributes[key] = intVal;
                    }
                }
                else if (value.is<float>())
                {
                    float floatVal = value.as<float>();
                    if (ImGui::InputFloat(key.c_str(), &floatVal))
                    {
                        _attributes[key] = floatVal;
                    }
                }
                break;
            case sol::type::boolean:
                {
                    bool boolVal = value.as<bool>();
                    if (ImGui::Checkbox(key.c_str(), &boolVal))
                    {
                        _attributes[key] = boolVal;
                    }
                }
                break;
            case sol::type::string:
                {
                    static std::map<std::string, std::string> stringBuffers;
                    std::string stringVal = value.as<std::string>();
                    
                    stringBuffers[key] = stringVal;
                    if (ImGui::InputText(key.c_str(), &stringBuffers[key][0], ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        _attributes[key] = stringBuffers[key];
                    }
                }
                break;
            case sol::type::userdata:
                // Assuming Vector2 is exposed as userdata
                if (value.is<Vector2>())
                {
                    Vector2 vec = value.as<Vector2>();
                    if (ImGui::InputFloat2(key.c_str(), &vec.x))
                    {
                        _attributes[key] = vec;
                    }
                }
                else if(value.is<Vector3>())
                {
                    Vector3 vec = value.as<Vector3>();
                    if (ImGui::InputFloat3(key.c_str(), &vec.x))
                    {
                        _attributes[key] = vec;
                    }
                }
                break;
            default:
                ImGui::TextDisabled("%s: Unsupported type", key.c_str());
        }

        ImGui::PopID();
    }
}
