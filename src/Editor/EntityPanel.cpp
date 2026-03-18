#include "Editor/EntityPanel.h"
#include "Editor/Editor.h"
#include "imgui/imgui.h"

EntityPanel::EntityPanel(std::weak_ptr<Editor> _editor)
: Panel(_editor)
, m_selectedEntity(-1)
, m_showAddToCategoryMenu(false)
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

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Entities & Properties", &m_showPanel))
    {
        // Use a table with 2 columns: left for entity list, right for properties
        if (ImGui::BeginTable("Entities", 2, ImGuiTableFlags_BordersInnerV))
        {
            ImGui::TableSetupColumn("Entities", ImGuiTableColumnFlags_WidthFixed, 250);
            ImGui::TableSetupColumn("Properties", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            // LEFT COLUMN: Entity List
            if (ImGui::BeginChild("EntityListPanel", ImVec2(0, 0), true))
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
                        if (ImGui::Selectable(("Entity" + std::to_string(entityId)).c_str(), selected))
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

                    DrawEntityProperties(m_selectedEntity);

                    auto l_editor = m_editor.lock();
                    auto l_registry = l_editor->GetScene()->GetRegistry();

                    if (ImGui::Button("Add to Category", ImVec2(-1, 0)))
                    {
                        m_showAddToCategoryMenu = !m_showAddToCategoryMenu;
                    }

                    if (m_showAddToCategoryMenu)
                    {
                        ImGui::Separator();
                        sol::table l_categories = l_registry->GetCategoryTable();
                        for (auto& l_pair : l_categories)
                        {
                            std::string l_name = l_pair.first.as<std::string>();
                            if (ImGui::MenuItem(l_name.c_str()))
                            {
                                auto l_category = l_registry->GetCategory(l_name).lock();
                                l_registry->AddToCategory(m_selectedEntity, l_category->GetSignature());
                                m_showAddToCategoryMenu = false;
                            }
                        }
                    }
                }
                else
                {
                    ImGui::TextDisabled("No entity selected");
                }

            }

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

void EntityPanel::DrawEntityProperties(int _entityId)
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

void EntityPanel::DrawCategoryAttributes(const std::string& _categoryName, sol::table& _attributes)
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
