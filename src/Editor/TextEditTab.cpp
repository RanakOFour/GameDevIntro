#include "Editor/TextEditTab.h"
#include "Editor/AutoCompleteTree.h"

#include "RanakEngine/Log.h"

TextEditTab::TextEditTab(std::weak_ptr<Editor> _editor)
: m_editor(_editor)
, m_textEditor()
, m_categoryPanel(_editor)
, m_rulesPanel(_editor)
, m_fileToEdit()
, m_size(1920, 1080)
, m_acTree()
{
    m_acTree = AutoCompleteTree::InitTree(_editor.lock()->GetEngineContents().core->GetLuaContext());
    m_categoryPanel.SetShown(true);
    // Set configs for text editor

    std::function<void()> l_textCallback(
        [this]()
        {
            m_fileToEdit->FlagReloaded();
        }
    );

    static std::string l_lastChangeWord = "";
    std::function<void(std::vector<TextEditor::Change>&)> l_changeCallback(
        [this](std::vector<TextEditor::Change>& _changes)
        {
            std::string l_isInsert = _changes.back().insert ? "Insert" : "Not insert";
            RE::Log::Message("Change callback text: " + l_isInsert + " " + _changes.back().text);
        }
    );

    m_textEditor.SetTransactionCallback([this](std::vector<TextEditor::Change>& _changes){ m_acTree.transactionCallback(_changes); });
    m_textEditor.SetChangeCallback([this](){ m_acTree.textCallback();});
    m_textEditor.SetLanguage(TextEditor::Language::Lua());
    TextEditor::AutoCompleteConfig* l_config = new TextEditor::AutoCompleteConfig();
    l_config->triggerOnTyping = true;
    l_config->triggerOnShortcut = true;

    l_config->callback = [this](TextEditor::AutoCompleteState& _state){ m_acTree.autocompleteCallback(_state); };

    m_textEditor.SetAutoCompleteConfig(l_config);
}

TextEditTab::~TextEditTab()
{
    m_fileToEdit.reset();
}

void DrawCategoryAttributes(std::string _categoryName, sol::table _attributes)
{
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

        // In the base-fields table, Field(default, opts) produces a wrapper table.
        // If the field is marked hidden, skip it; otherwise unwrap to the default value for display.
        if (l_value.get_type() == sol::type::table)
        {
            sol::table l_tbl = l_value.as<sol::table>();
            sol::optional<bool> l_isField = l_tbl["__isField"];
            if (l_isField.has_value() && *l_isField)
            {
                sol::optional<sol::table> l_opts = l_tbl["opts"];
                if (l_opts.has_value())
                {
                    sol::optional<bool> l_hidden = (*l_opts)["hidden"];
                    if (l_hidden.has_value() && *l_hidden)
                    {
                        ImGui::PopID();
                        continue;
                    }
                }
                // Not hidden — unwrap to the default value so normal display works.
                l_value = l_tbl["default"];
            }
        }

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

            ImGui::Text(l_value.as<std::string>().c_str());
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

void TextEditTab::Draw()
{
    // Pin the window below the main menu bar every frame so the menu bar
    // is always visible and clickable above the text editor.
    ImGuiIO& l_io = ImGui::GetIO();
    float l_menuH = ImGui::GetFrameHeight();
    m_size = ImVec2(l_io.DisplaySize.x, l_io.DisplaySize.y - l_menuH);
    ImGui::SetNextWindowPos(ImVec2(0.0f, l_menuH), ImGuiCond_Always);
    ImGui::SetNextWindowSize(m_size, ImGuiCond_Always);

    bool l_showTab = true;
    if(ImGui::Begin("TextEdit", &l_showTab, ImGuiWindowFlags_NoTitleBar |
                                            ImGuiWindowFlags_NoMove |
                                            ImGuiWindowFlags_NoResize |
                                            ImGuiWindowFlags_NoBringToFrontOnFocus |
                                            ImGuiWindowFlags_NoFocusOnAppearing))
    {
        if(ImGui::BeginTable("TabTable", 3, ImGuiTableFlags_SizingFixedFit))
        {
            ImGui::TableSetupColumn("Files", ImGuiTableColumnFlags_WidthFixed, m_size.x / 5);
            ImGui::TableSetupColumn("Text Editor", ImGuiTableColumnFlags_WidthFixed, (3 * m_size.x) / 5);
            ImGui::TableSetupColumn("Properties", ImGuiTableColumnFlags_WidthFixed, m_size.x / 5);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            if(ImGui::BeginTabBar("FileTabBar"))
            {
                if(ImGui::BeginTabItem("Categories"))
                {
                    m_categoryPanel.SetShown(true);
                    m_rulesPanel.SetShown(false);
                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem("Rules"))
                {
                    m_categoryPanel.SetShown(false);
                    m_rulesPanel.SetShown(true);
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            m_categoryPanel.DrawAsChild(ImGuiChildFlags_None);
            m_rulesPanel.DrawAsChild(ImGuiChildFlags_None);

            ImGui::TableSetColumnIndex(1);

            if(ImGui::BeginChild("TextEditor", ImVec2(0, 0)))
            {
                auto l_catFile = m_categoryPanel.GetSelectedFile().lock();
                if(l_catFile.get() != m_fileToEdit.get())
                {
                    m_textEditor.ClearText();
                    m_fileToEdit = l_catFile;
                    m_textEditor.SetText(m_fileToEdit->GetCode());
                }

                if(m_fileToEdit != nullptr)
                {
                    if(m_fileToEdit->GetReloaded())
                    {
                        ImGui::Text("%s *", m_fileToEdit->GetName().c_str());
                        ImGui::SameLine();
                        if(ImGui::Button("Save"))
                        {
                            SaveCurrentFile();
                        }
                    }
                    else
                    {
                        ImGui::Text("%s", m_fileToEdit->GetName().c_str());
                    }

                    m_textEditor.Render(m_fileToEdit->GetName().c_str(), ImVec2((3 * m_size.x) / 5, 900), true);
                }
            }
            ImGui::EndChild();

            ImGui::TableSetColumnIndex(2);

            if(m_fileToEdit != nullptr)
            {
                if(ImGui::BeginChild("Properties", ImVec2(0, 0)))
                {
                    // Resolve the category by name from the LuaContext rather than
                    // via a forward pointer stored on the file itself.
                    auto l_category = RE::Core::LuaContext::Instance().lock()
                                        ->GetCategory(m_fileToEdit->GetName()).lock();

                    if (l_category)
                    {
                        std::string l_categoryName = l_category->GetName();
                        sol::table l_categoryData = l_category->GetBaseData();

                        ImGui::Text(l_categoryName.c_str());
                        ImGui::Indent();
                        DrawCategoryAttributes(l_categoryName, l_categoryData);
                        ImGui::Unindent();
                    }
                }
                ImGui::EndChild();
            }

            ImGui::EndTable();
        }
    }

    ImGui::End();
}

void TextEditTab::SetFile(std::weak_ptr<RE::Asset::LuaFile> _file)
{
    assert(_file.lock() != nullptr);

    m_fileToEdit = _file.lock();
}

void TextEditTab::SaveCurrentFile()
{
    if (m_fileToEdit == nullptr)
        return;

    // Write new code to disk
    std::string l_newCode = m_textEditor.GetText();
    m_fileToEdit->SetCode(l_newCode);
    m_fileToEdit->Save();

    auto l_category = RE::Core::LuaContext::Instance().lock()
                        ->GetCategory(m_fileToEdit->GetName()).lock();
    if (!l_category) return;

    std::string l_catName      = l_category->GetName();
    std::bitset<1024> l_catSig = l_category->GetSignature();

    auto l_scene      = m_editor.lock()->GetEngineContents().core->GetScene().lock();
    auto& l_registry  = l_scene->GetRegistry();  // reference, not a copy
    auto l_luaContext = RE::Core::LuaContext::Instance().lock();

    // Cache each entity's current field values and remove it from the category.
    std::vector<int> l_entities = l_registry.GetEntitiesWith(l_catSig);
    std::vector<std::pair<int, sol::table>> l_cachedData;

    for (int l_entity : l_entities)
    {
        sol::table l_cache    = l_luaContext->CreateTable();
        sol::table l_attribs  = l_scene->GetSceneTable().traverse_raw_get<sol::table>("Entities", l_entity, "attributes");
        sol::table l_catData  = l_attribs.raw_get<sol::table>(l_catName);

        for (auto& l_pair : l_catData.pairs())
        {
            l_cache.raw_set(l_pair.first, l_pair.second);
        }

        l_cachedData.push_back({l_entity, l_cache});
        l_scene->RemoveFromCategory(l_entity, l_catName);
    }

    m_fileToEdit->Reload();

    // Re-add each entity and restore any fields that still exist in the new definition.
    for (auto& [l_entity, l_cache] : l_cachedData)
    {
        l_scene->AddToCategory(l_entity, l_catName);

        sol::table l_attribs    = l_scene->GetSceneTable().traverse_raw_get<sol::table>("Entities", l_entity, "attributes");
        sol::table l_newCatData = l_attribs.raw_get<sol::table>(l_catName);

        for (auto& l_pair : l_cache.pairs())
        {
            // Only restore if the new definition still has this field
            sol::optional<sol::object> l_existing = l_newCatData.raw_get<sol::optional<sol::object>>(l_pair.first);
            if (l_existing.has_value())
            {
                l_newCatData.raw_set(l_pair.first, l_pair.second);
            }
        }
    }

    RE::Log::Message("Category saved and reloaded: " + m_fileToEdit->GetPath());
}