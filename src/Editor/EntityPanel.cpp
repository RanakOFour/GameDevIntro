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

    ImGui::SetNextWindowSize(ImVec2(800, 600));
    if (ImGui::Begin("Entities & Properties", &m_showPanel))
    {
        // Use a table with 2 columns: left for entity list, right for properties
        if (ImGui::BeginTable("Entities", 2, ImGuiTableFlags_BordersInnerV))
        {
            ImGui::TableSetupColumn("Entities", ImGuiTableColumnFlags_WidthFixed, 300);
            ImGui::TableSetupColumn("Properties", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            auto l_editor = m_editor.lock();
            auto l_registry = l_editor->GetScene()->GetRegistry();

            // LEFT COLUMN: Entity List
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

            // RIGHT COLUMN: Properties
            ImGui::TableSetColumnIndex(1);

            if (ImGui::BeginChild("Properties", ImVec2(0, 0), true))
            {
                if (m_selectedEntity > -1)
                {
                    ImGui::Text("Entity ID: %d", m_selectedEntity);
                    ImGui::Separator();

                    std::string l_entityIDString = std::to_string(m_selectedEntity);

                    if(m_stringValueMap.find(l_entityIDString) == m_stringValueMap.end())
                    {
                        m_stringValueMap[l_entityIDString] = l_registry->GetEntityName(m_selectedEntity);
                    }

                    if(ImGui::InputText("Name", &m_stringValueMap[l_entityIDString], ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        sol::table l_entityTable = l_registry->GetEntityTable().raw_get<sol::table>(m_selectedEntity);
                        l_entityTable["name"] = m_stringValueMap[l_entityIDString];
                    }

                    ImGui::Separator();

                    DrawEntityProperties(m_selectedEntity);

                    if (ImGui::Button("Add to Category", ImVec2(-1, 0)))
                    {
                        m_showAddToCategoryMenu = !m_showAddToCategoryMenu;
                    }

                    if (m_showAddToCategoryMenu)
                    {
                        ImGui::Separator();

                        // I love cache!!! Bleh :3
                        std::stringstream l_categories(l_editor->GetEngineContents()
                                                          .core->GetLuaContext()
                                                               ->GetCategoryNames());
                        std::string l_categoryName;

                        sol::table l_entityData = l_registry->GetEntityAttributes(m_selectedEntity);

                        while(std::getline(l_categories, l_categoryName, ';'))
                        {
                            bool l_showCategory = true;

                            // Rule out categories that are already on the entity
                            for(auto& l_pair : l_entityData)
                            {
                                std::string l_entityCatName = l_pair.first.as<std::string>();
                                if(l_entityCatName == l_categoryName)
                                {
                                    l_showCategory = false;
                                }
                            }

                            //Only show unknown categories
                            if(l_showCategory)
                            {
                                if(ImGui::MenuItem(l_categoryName.c_str()))
                                {
                                    // Peak cache optimisation. A s_ptr<LuaContext> would probably be best
                                    auto l_category = l_editor->GetEngineContents()
                                                         .core->GetLuaContext()
                                                              ->GetCategory(l_categoryName).lock();

                                    l_registry->AddToCategory(m_selectedEntity, l_category->GetSignature());
                                    m_showAddToCategoryMenu = false;
                                }
                            }
                        }
                    }
                }
                else
                {
                    ImGui::Text("No entity selected");
                }
            }
            
            // Program fails an assert when this is inside the above selection branch
            ImGui::EndChild();

            ImGui::EndTable();
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

void EntityPanel::DrawEntityProperties(int _entityId)
{
    auto l_registry = m_editor.lock()->GetScene()->GetRegistry();

    sol::table l_entityData = l_registry->GetEntityAttributes(_entityId);
        
    // Display each category and its attributes
    for (auto& l_pair : l_entityData)
    {
        std::string l_categoryName = l_pair.first.as<std::string>();
        sol::table l_attributes = l_pair.second.as<sol::table>();

        if (ImGui::CollapsingHeader(l_categoryName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Indent();
            DrawCategoryAttributes(_entityId, l_categoryName, l_attributes);
            ImGui::Unindent();
        }
    }
}

void EntityPanel::DrawCategoryAttributes(const int& _entityId, const std::string& _categoryName, sol::table& _attributes)
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

                    if(m_stringValueMap.find(l_key) == m_stringValueMap.end())
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
                else if(l_value.is<Vector3>())
                {
                    Vector3 l_val = l_value.as<Vector3>();
                    if (ImGui::DragFloat3(l_property.c_str(), &l_val.x, 0.1f, -std::numeric_limits<float>().infinity(), std::numeric_limits<float>().infinity(), "%.3f"))
                    {
                        _attributes[l_property] = l_val;
                    }
                }
                else if(l_value.is<Vector4>())
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
