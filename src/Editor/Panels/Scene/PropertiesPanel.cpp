#include "Editor/Panels/Scene/PropertiesPanel.h"
#include "Editor/Core/Editor.h"
#include "Editor/Tabs/SceneEditTab.h"
#include "Editor/Panels/Scene/AssetBrowserPanel.h"
#include "Editor/Core/UndoManager.h"

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
    std::string l_categoryToRemove;

    for (auto& l_pair : l_entityData)
    {
        std::string l_categoryName = l_pair.first.as<std::string>();
        sol::table l_attributes = l_pair.second.as<sol::table>();

        ImGui::PushID(l_categoryName.c_str());

        bool l_headerOpen = ImGui::CollapsingHeader("##cat", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap);

        // Draw remove button on the same line (except for Transform which is mandatory)
        ImGui::SameLine();
        ImGui::TextUnformatted(l_categoryName.c_str());
        if (l_categoryName != "Transform")
        {
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 10.0f);
            if (ImGui::SmallButton("X"))
            {
                l_categoryToRemove = l_categoryName;
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Remove from %s", l_categoryName.c_str());
            }
        }

        if (l_headerOpen)
        {
            ImGui::Indent();
            DrawCategoryAttributes(_id, l_categoryName, l_attributes);
            ImGui::Unindent();
        }

        ImGui::PopID();
    }

    // Process removal outside the iteration to avoid invalidating the table
    if (!l_categoryToRemove.empty())
    {
        auto l_scene = m_editor.GetEngineContents().core->GetScene().lock();
        if (l_scene)
        {
            // Snapshot field values for undo
            sol::table l_catData = l_entityData.raw_get<sol::table>(l_categoryToRemove);
            auto l_catDataCopy = std::make_shared<std::vector<std::pair<std::string, sol::object>>>();
            for (auto& l_fp : l_catData)
            {
                l_catDataCopy->emplace_back(l_fp.first.as<std::string>(), l_fp.second);
            }

            l_scene->RemoveFromCategory(_id, l_categoryToRemove);

            int l_entityId = _id;
            std::string l_catName = l_categoryToRemove;
            Editor* l_edRaw = &m_editor;
            m_editor.GetUndoManager().PushCommand(
                std::make_unique<LambdaCommand>("Remove " + l_catName,
                    [l_edRaw, l_entityId, l_catName]()
                    {
                        auto l_sc = l_edRaw->GetEngineContents().core->GetScene().lock();
                        if (l_sc)
                        { 
                            l_sc->RemoveFromCategory(l_entityId, l_catName);
                        }
                    },
                    [l_edRaw, l_entityId, l_catName, l_catDataCopy]()
                    {
                        auto l_sc = l_edRaw->GetEngineContents().core->GetScene().lock();
                        if (l_sc)
                        {
                            l_sc->AddToCategory(l_entityId, l_catName);
                            sol::table l_attrs = l_sc->GetRegistry().GetEntityAttributes(l_entityId);
                            sol::object l_catObj = l_attrs.raw_get<sol::object>(l_catName);
                            if (l_catObj.valid() && l_catObj.get_type() == sol::type::table)
                            {
                                sol::table l_cat = l_catObj.as<sol::table>();
                                for (auto& [l_fn, l_fv] : *l_catDataCopy)
                                {
                                    l_cat[l_fn] = l_fv;
                                }
                            }
                        }
                    }
                )
            );
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
        bool l_isField = false;
        bool l_isColour = false;
        bool l_isEnum = false;
        std::vector<std::string> l_enumOptions;

        // Skip properties marked hidden via Field(default, { hidden = true }) in the category definition.
        if (l_baseFields.valid())
        {
            sol::object l_baseMeta = l_baseFields.raw_get<sol::object>(l_property.c_str());
            if (l_baseMeta.get_type() == sol::type::table)
            {
                sol::table l_metaTable = l_baseMeta.as<sol::table>();
                sol::optional<bool> l_isFieldOpt = l_metaTable["__isField"];
                if (l_isFieldOpt.has_value() && *l_isFieldOpt)
                {
                    l_isField = true;
                    sol::optional<sol::table> l_opts = l_metaTable["opts"];
                    if (l_opts.has_value())
                    {
                        sol::optional<bool> l_hidden = (*l_opts)["hidden"];
                        if (l_hidden.has_value() && *l_hidden)
                        {
                            continue;
                        }

                        sol::optional<bool> l_isEnumOpt = (*l_opts)["isEnum"];
                        if (l_isEnumOpt.has_value() && *l_isEnumOpt)
                        {
                            l_isEnum = true;
                            sol::optional<sol::table> l_enumTbl = (*l_opts)["enumOptions"];
                            if (l_enumTbl.has_value())
                            {
                                for (auto& l_ep : l_enumTbl->as<sol::table>())
                                {
                                    l_enumOptions.push_back(l_ep.second.as<std::string>());
                                }
                            }

                            std::string l_value = l_metaTable["default"].get<std::string>();

                            if(ImGui::BeginCombo((l_property + "##combo").c_str(), l_value.c_str()))
                            {
                                for (const std::string& l_option : l_enumOptions)
                                {
                                    bool l_selected = (l_value == l_option);
                                    if (ImGui::Selectable(l_option.c_str(), l_selected))
                                    {
                                        l_metaTable.raw_set("default", l_option);
                                    }
                                }
                                ImGui::EndCombo();
                            }

                            continue;
                        }

                        sol::optional<bool> l_isColourOpt = (*l_opts)["isColour"];
                        if(l_isColourOpt.has_value() && *l_isColourOpt)
                        {
                            l_isColour = true;
                        }
                    }
                }
            }
        }

        // Create a unique ID for this property
        ImGui::PushID((_categoryName + "::" + l_property).c_str());

        sol::object l_value = _attributes.raw_get<sol::object>(l_property.c_str());

        // Check for Field wrapper and unwrap to the default value for display.
        if(l_value.is<sol::table>())
        {
            sol::table l_tbl = l_value.as<sol::table>();
            sol::optional<bool> l_fieldFlag = l_tbl.raw_get<sol::optional<bool>>("__isField");
            if (l_fieldFlag.has_value() && *l_fieldFlag)
            {
                l_value = l_tbl.raw_get<sol::object>("default");
            }
        }

        switch (l_value.get_type())
        {
        case sol::type::number:
            if (l_value.is<int>())
            {
                int l_old = l_value.as<int>();
                int l_val = l_old;
                if (ImGui::InputInt(l_property.c_str(), &l_val))
                {
                    _attributes[l_property] = l_val;
                    int l_new = l_val;
                    sol::table l_tbl = _attributes;
                    std::string l_prop = l_property;
                    m_editor.GetUndoManager().PushCommand(
                        std::make_unique<LambdaCommand>("Edit " + l_prop,
                            [l_tbl, l_prop, l_new]()
                            {
                                std::remove_const<sol::table>::type& l_tblNonConst = 
                                    const_cast<std::remove_const<sol::table>::type&>(l_tbl);
                                l_tblNonConst[l_prop] = l_new;
                            },
                            [l_tbl, l_prop, l_old]()
                            {
                                std::remove_const<sol::table>::type& l_tblNonConst = 
                                    const_cast<std::remove_const<sol::table>::type&>(l_tbl);
                                l_tblNonConst[l_prop] = l_old;
                            }));
                }
            }
            else if (l_value.is<float>())
            {
                float l_old = l_value.as<float>();
                float l_val = l_old;
                if (ImGui::InputFloat(l_property.c_str(), &l_val, 0.1f, 0.0f, "%.3f"))
                {
                    _attributes[l_property] = l_val;
                    float l_new = l_val;
                    sol::table l_tbl = _attributes;
                    std::string l_prop = l_property;
                    m_editor.GetUndoManager().PushCommand(
                        std::make_unique<LambdaCommand>("Edit " + l_prop,
                            [l_tbl, l_prop, l_new]()
                            {
                                std::remove_const<sol::table>::type& l_tblNonConst = 
                                    const_cast<std::remove_const<sol::table>::type&>(l_tbl);
                                l_tblNonConst[l_prop] = l_new;
                            },
                            [l_tbl, l_prop, l_old]()
                            {
                                std::remove_const<sol::table>::type& l_tblNonConst = 
                                    const_cast<std::remove_const<sol::table>::type&>(l_tbl);
                                l_tblNonConst[l_prop] = l_old;
                            }));
                }
            }
            break;
        case sol::type::boolean:
        {
            bool l_old = l_value.as<bool>();
            bool l_val = l_old;
            if (ImGui::Checkbox(l_property.c_str(), &l_val))
            {
                _attributes[l_property] = l_val;
                bool l_new = l_val;
                sol::table l_tbl = _attributes;
                std::string l_prop = l_property;
                m_editor.GetUndoManager().PushCommand(
                    std::make_unique<LambdaCommand>("Edit " + l_prop,
                        [l_tbl, l_prop, l_new]()
                        {
                            std::remove_const<sol::table>::type& l_tblNonConst = 
                                const_cast<std::remove_const<sol::table>::type&>(l_tbl);
                            l_tblNonConst[l_prop] = l_new;
                        },
                        [l_tbl, l_prop, l_old]()
                        {
                            std::remove_const<sol::table>::type& l_tblNonConst = 
                                const_cast<std::remove_const<sol::table>::type&>(l_tbl);
                            l_tblNonConst[l_prop] = l_old;
                        }));
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
            
            std::string l_oldStr = l_value.as<std::string>();
            bool l_committed = ImGui::InputText(l_property.c_str(), &m_stringValueMap[l_key], ImGuiInputTextFlags_EnterReturnsTrue);
            l_committed |= ImGui::IsItemDeactivatedAfterEdit();
            if (l_committed)
            {
                _attributes[l_property] = m_stringValueMap[l_key];
                std::string l_newStr = m_stringValueMap[l_key];
                sol::table l_tbl = _attributes;
                std::string l_prop = l_property;
                std::string l_k = l_key;
                auto l_mapPtr = &m_stringValueMap;
                m_editor.GetUndoManager().PushCommand(
                    std::make_unique<LambdaCommand>("Edit " + l_prop,
                        [l_tbl, l_prop, l_newStr, l_mapPtr, l_k]()
                        {
                            std::remove_const<sol::table>::type& l_tblNonConst = 
                                const_cast<std::remove_const<sol::table>::type&>(l_tbl);
                            l_tblNonConst[l_prop] = l_newStr;
                            (*l_mapPtr)[l_k] = l_newStr;
                        },
                        [l_tbl, l_prop, l_oldStr, l_mapPtr, l_k]()
                        {
                            std::remove_const<sol::table>::type& l_tblNonConst = 
                                const_cast<std::remove_const<sol::table>::type&>(l_tbl);
                            l_tblNonConst[l_prop] = l_oldStr;
                            (*l_mapPtr)[l_k] = l_oldStr;
                        }));
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
            if (l_value.is<Vector2>())
            {
                Vector2 l_old = l_value.as<Vector2>();
                Vector2 l_val = l_old;
                if (ImGui::InputFloat2(l_property.c_str(), &l_val.x, "%.3f"))
                {
                    _attributes[l_property] = l_val;
                    Vector2 l_new = l_val;
                    sol::table l_tbl = _attributes;
                    std::string l_prop = l_property;
                    m_editor.GetUndoManager().PushCommand(
                        std::make_unique<LambdaCommand>("Edit " + l_prop,
                            [l_tbl, l_prop, l_new]()
                            {
                                std::remove_const<sol::table>::type& l_tblNonConst = 
                                    const_cast<std::remove_const<sol::table>::type&>(l_tbl);
                                l_tblNonConst[l_prop] = l_new;
                            },
                            [l_tbl, l_prop, l_old]()
                            {
                                std::remove_const<sol::table>::type& l_tblNonConst = 
                                    const_cast<std::remove_const<sol::table>::type&>(l_tbl);
                                l_tblNonConst[l_prop] = l_old;
                            }));
                }
            }
            else if (l_value.is<Vector3>())
            {
                Vector3 l_old = l_value.as<Vector3>();
                Vector3 l_val = l_old;
                if ((l_isColour && ImGui::ColorEdit3(l_property.c_str(), &l_val.x)) || ImGui::InputFloat3(l_property.c_str(), &l_val.x, "%.3f"))
                {
                    _attributes[l_property] = l_val;
                    Vector3 l_new = l_val;
                    sol::table l_tbl = _attributes;
                    std::string l_prop = l_property;
                    m_editor.GetUndoManager().PushCommand(
                        std::make_unique<LambdaCommand>("Edit " + l_prop,
                            [l_tbl, l_prop, l_new]()
                            {
                                std::remove_const<sol::table>::type& l_tblNonConst = 
                                    const_cast<std::remove_const<sol::table>::type&>(l_tbl);
                                l_tblNonConst[l_prop] = l_new;
                            },
                            [l_tbl, l_prop, l_old]()
                            {
                                std::remove_const<sol::table>::type& l_tblNonConst = 
                                    const_cast<std::remove_const<sol::table>::type&>(l_tbl);
                                l_tblNonConst[l_prop] = l_old;
                            }));
                }
            }
            else if (l_value.is<Vector4>())
            {
                Vector4 l_old = l_value.as<Vector4>();
                Vector4 l_val = l_old;
                if ((l_isColour && ImGui::ColorEdit4(l_property.c_str(), &l_val.x)) ||
                    (!l_isColour && ImGui::InputFloat4(l_property.c_str(), &l_val.x, "%.3f")))
                {
                    _attributes[l_property] = l_val;

                    Vector4 l_new = l_val;
                    sol::table l_tbl = _attributes;
                    std::string l_prop = l_property;
                    m_editor.GetUndoManager().PushCommand(
                        std::make_unique<LambdaCommand>("Edit " + l_prop,
                            [l_tbl, l_prop, l_new]()
                            {
                                std::remove_const<sol::table>::type& l_tblNonConst = 
                                    const_cast<std::remove_const<sol::table>::type&>(l_tbl);
                                l_tblNonConst[l_prop] = l_new;
                            },
                            [l_tbl, l_prop, l_old]()
                            {
                                std::remove_const<sol::table>::type& l_tblNonConst = 
                                    const_cast<std::remove_const<sol::table>::type&>(l_tbl);
                                l_tblNonConst[l_prop] = l_old;
                            }));
                }
            }
            break;
        case sol::type::table:
            if (l_isColour)
            {
                // Colour stored as a plain {x,y,z,w} Lua table (e.g. from deserialization)
                sol::table l_tbl = l_value.as<sol::table>();
                Vector4 l_val(
                    l_tbl.get_or("x", 1.0f),
                    l_tbl.get_or("y", 1.0f),
                    l_tbl.get_or("z", 1.0f),
                    l_tbl.get_or("w", 1.0f)
                );
                if (ImGui::ColorEdit4(l_property.c_str(), &l_val.x))
                {
                    l_tbl.raw_set("x", l_val.x);
                    l_tbl.raw_set("y", l_val.y);
                    l_tbl.raw_set("z", l_val.z);
                    l_tbl.raw_set("w", l_val.w);
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

    // Invalidate string cache when selection changes
    if (l_selectedEntity != m_lastSelectedEntity)
    {
        m_stringValueMap.clear();
        m_lastSelectedEntity = l_selectedEntity;
    }

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
            std::string l_newName = m_entityNameMap[l_selectedEntity];
            std::string l_oldName = l_entityTable.raw_get_or<std::string>("name", "");
            l_entityTable["name"] = l_newName;

            int l_eid = l_selectedEntity;
            auto* l_nameMap = &m_entityNameMap;
            m_editor.GetUndoManager().PushCommand(
                std::make_unique<LambdaCommand>("Rename Entity",
                    [l_entityTable, l_eid, l_newName, l_nameMap]() {
                        std::remove_const<sol::table>::type& l_entityTableNonConst = 
                        const_cast<std::remove_const<sol::table>::type&>(l_entityTable);
                        
                        l_entityTableNonConst["name"] = l_newName;
                        (*l_nameMap)[l_eid] = l_newName;
                    },
                    [l_entityTable, l_eid, l_oldName, l_nameMap]() {
                        std::remove_const<sol::table>::type& l_entityTableNonConst = 
                        const_cast<std::remove_const<sol::table>::type&>(l_entityTable);
                        
                        l_entityTableNonConst["name"] = l_oldName;
                        (*l_nameMap)[l_eid] = l_oldName;
                    }));
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

                if (l_showCategory)
                {
                    if (ImGui::MenuItem(l_categoryName.c_str()))
                    {
                        auto l_scene = m_editor.GetEngineContents().core->GetScene().lock();
                        if (l_scene)
                        {
                            l_scene->AddToCategory(l_selectedEntity, l_categoryName);

                            int l_eid = l_selectedEntity;
                            std::string l_catName = l_categoryName;
                            Editor* l_edRaw = &m_editor;
                            m_editor.GetUndoManager().PushCommand(
                                std::make_unique<LambdaCommand>("Add " + l_catName,
                                    [l_edRaw, l_eid, l_catName]() {
                                        auto l_sc = l_edRaw->GetEngineContents().core->GetScene().lock();
                                        if (l_sc) l_sc->AddToCategory(l_eid, l_catName);
                                    },
                                    [l_edRaw, l_eid, l_catName]() {
                                        auto l_sc = l_edRaw->GetEngineContents().core->GetScene().lock();
                                        if (l_sc) l_sc->RemoveFromCategory(l_eid, l_catName);
                                    }));
                        }
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