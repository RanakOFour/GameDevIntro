#include "Editor/TextEditTab.h"

#include "RanakEngine/Log.h"

TextEditTab::TextEditTab(std::weak_ptr<Editor> _editor)
: m_editor(_editor)
, m_textEditor()
, m_categoryPanel(_editor)
, m_rulesPanel(_editor)
, m_fileToEdit()
, m_size(1920, 1080)
{
    m_categoryPanel.SetShown(true);
    // Set configs for text editor

    std::function<void()> l_textCallback([this](){ m_fileToEdit->FlagReloaded(); });

    m_textEditor.SetChangeCallback(l_textCallback);
    m_textEditor.SetLanguage(TextEditor::Language::Lua());
    TextEditor::AutoCompleteConfig* l_config = new TextEditor::AutoCompleteConfig();
    l_config->triggerOnTyping = true;
    l_config->triggerOnShortcut = true;
    std::function<void(TextEditor::AutoCompleteState&)> l_configCallback([](TextEditor::AutoCompleteState& _state)
    {
        // Search current word for keywords (lib names, functions, etc.)
        RE::Log::Message("Current search term: " + _state.searchTerm);
    });

    l_config->callback = l_configCallback;

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
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(m_size);
    bool l_showTab = true;
    if(ImGui::Begin("TextEdit", &l_showTab, ImGuiWindowFlags_NoTitleBar))
    {
        if(ImGui::BeginTable("TabTable", 3, ImGuiTableFlags_SizingFixedFit))
        {
            ImGui::TableSetupColumn("Files", ImGuiTableColumnFlags_WidthFixed, m_size.x / 5);
            ImGui::TableSetupColumn("Text Editor", ImGuiTableColumnFlags_WidthFixed, (3 * m_size.x) / 5);
            ImGui::TableSetupColumn("Properties", ImGuiTableColumnFlags_WidthFixed, m_size.x / 5);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            m_categoryPanel.DrawAsChild(ImGuiChildFlags_None);

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

                ImGui::EndChild();
            }

            ImGui::TableSetColumnIndex(2);

            if(m_fileToEdit != nullptr && ImGui::BeginChild("Properties", ImVec2(0, 0)))
            {
                auto l_category = m_fileToEdit->GetCategory().lock();
                std::string l_categoryName = l_category->GetName();
                sol::table l_categoryData = l_category->GetBaseData();
                
                ImGui::Text(l_categoryName.c_str());
                ImGui::Indent();
                DrawCategoryAttributes(l_categoryName, l_categoryData);
                ImGui::Unindent();

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
    if (m_fileToEdit != nullptr)
    {
        // Get the text from the editor
        std::string l_newCode = m_textEditor.GetText();
        
        // Save to the file
        m_fileToEdit->SetCode(l_newCode);
        m_fileToEdit->Save();
        

        auto l_newCategory = m_fileToEdit->GetCategory()
                                         .lock();

        std::string l_oldName = l_newCategory->GetName();

        // Clear the reload flag
        m_fileToEdit->Reload();
        
        std::bitset<1024> l_catSignature = l_newCategory->GetSignature();

        auto l_scene = m_editor.lock()->GetEngineContents()
                                      .core
                                      ->GetScene()
                                      .lock();

        auto l_registry = l_scene->GetRegistry();

        // Remove category to delete all old data and readd category
        std::vector<int> l_entitiesInCategory = l_registry.GetEntitiesWith(l_catSignature);

        for(int l_entity : l_entitiesInCategory)
        {
            sol::table l_cacheTable = RE::Core::LuaContext::Instance().lock()->CreateTable();
            sol::table l_entityData = l_scene->GetSceneTable().traverse_raw_get<sol::table>("Entities", l_entity, "attributes");

            auto l_dataPairs = l_entityData.raw_get<sol::table>(l_oldName).pairs();

            // Store all category data
            for(auto& l_pair : l_dataPairs)
            {
                l_cacheTable.raw_set(l_pair.first, l_pair.second);
            }

            l_scene->AddEntityToCategory(l_entity, l_catSignature);
            
            // Reapply entity data if property still exists
            for(auto& l_pair : l_dataPairs)
            {
                sol::optional<sol::object> l_entry = l_entityData.raw_get<sol::optional<sol::object>>(l_pair.first);
                
                if(l_entry.has_value())
                {
                    l_entityData.raw_set(l_pair.first,  l_pair.second);
                }
            }
        }
        
        RE::Log::Message("Category saved to file: " + m_fileToEdit->GetPath());
    }
}