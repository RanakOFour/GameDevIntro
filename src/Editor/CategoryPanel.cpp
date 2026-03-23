#include "Editor/CategoryPanel.h"
#include "Editor/Editor.h"

#include "imgui/imgui.h"

// String compatible functions for ImGui
#include "imgui/misc/cpp/imgui_stdlib.h"

#define USE_STD_FILESYSTEM 1
#include "imguiFileDialog/ImGuiFileDialog.h"

#include <algorithm>
#include <sstream>
#include <string>

CategoryPanel::CategoryPanel(std::weak_ptr<Editor> _editor)
: Panel(_editor)
, m_showCreateDialog(false)
, m_showLoadDialog(false)
, m_newCategoryName("")
, m_selectedCategoryFilter("")
, m_loadCategoryFilePath("./resources/Categories/")
, m_selectedCategory(-1)
{
    RefreshCategoryList();
    m_textEditor = std::make_shared<TextEditor>();

    std::function<void()> l_textCallback([this](){ m_selectedCategoryOrigin->FlagReloaded(); });

    m_textEditor->SetChangeCallback(l_textCallback);
    m_textEditor->SetLanguage(TextEditor::Language::Lua());
    TextEditor::AutoCompleteConfig* l_config = new TextEditor::AutoCompleteConfig();
    l_config->triggerOnTyping = true;
    l_config->triggerOnShortcut = true;
    std::function<void(TextEditor::AutoCompleteState&)> l_configCallback([](TextEditor::AutoCompleteState& _state)
    {
        // Search current word for keywords (lib names, functions, etc.)
        RE::Log::Message("Current search term: " + _state.searchTerm);
    });

    l_config->callback = l_configCallback;

    m_textEditor->SetAutoCompleteConfig(l_config);
}

CategoryPanel::~CategoryPanel()
{
}

void CategoryPanel::TextEditorCallback()
{
    m_selectedCategoryOrigin->FlagReloaded();
}

void CategoryPanel::RefreshCategoryList()
{
    m_availableCategories.clear();

    auto l_context = m_editor.lock()->GetEngineContents()
                               .core->GetLuaContext();
    
    std::stringstream l_catNames(l_context->GetCategoryNames());
    std::string l_segment;

    while(std::getline(l_catNames, l_segment, ';'))
    {
        m_availableCategories.push_back(l_segment);
    }

    std::sort(m_availableCategories.begin(), m_availableCategories.end());
}

void CategoryPanel::Draw()
{
    if (!m_showPanel) return;

    RefreshCategoryList();
    
    if (ImGui::Begin("Categories", &m_showPanel))
    {
        if(ImGui::BeginTable("Categories", 2, ImGuiTableFlags_BordersInnerV))
        {
            ImGui::TableSetupColumn("Categories", ImGuiTableColumnFlags_WidthFixed, 300);
            ImGui::TableSetupColumn("Properties", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);


            // Category list
            if(ImGui::BeginChild("CategoryListPanel", ImVec2(0, 0), true))
            {
                float buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
                // Create and Load buttons
                if (ImGui::Button("+ Create", ImVec2(buttonWidth, 0)))
                {
                    m_showCreateDialog = true;
                }

                ImGui::SameLine();

                if (ImGui::Button("Load", ImVec2(buttonWidth, 0)))
                {
                    m_showLoadDialog = true;
                }

                ImGui::Separator();

                // Search/Filter
                ImGui::InputTextWithHint("##", "Search categories...", &m_selectedCategoryFilter);

                ImGui::Separator();

                // Category list
                if (ImGui::BeginChild("CategoryList", ImVec2(0, 0), true))
                {
                    for (int i = 0; i < m_availableCategories.size(); i++)
                    {
                        auto& l_category = m_availableCategories[i];

                        if (m_selectedCategoryFilter.size() > 0)
                        {
                            std::string filter(m_selectedCategoryFilter);
                            if (l_category.find(filter) == std::string::npos)
                            {
                                continue;
                            }
                        }

                        bool l_selected = (m_selectedCategory == i);
                        if(ImGui::Selectable(l_category.c_str(), l_selected))
                        {
                            SelectCategory(i);
                        };
                    }
                    
                    ImGui::EndChild();
                }

                ImGui::EndChild();
            }

            ImGui::TableSetColumnIndex(1);

            if(m_selectedCategory != -1 && ImGui::BeginChild("Properties", ImVec2(0, 0), true))
            {
                if(m_selectedCategoryOrigin != nullptr)
                {
                    if(m_selectedCategoryOrigin->GetReloaded())
                    {
                        ImGui::Text("%s *", m_availableCategories[m_selectedCategory].c_str());
                        ImGui::SameLine();
                        if(ImGui::Button("Save"))
                        {
                            SaveCategoryToFile();
                        }
                    }
                    else
                    {
                        ImGui::Text("%s", m_availableCategories[m_selectedCategory].c_str());
                    }

                    m_textEditor->Render(m_availableCategories[m_selectedCategory].c_str(), ImVec2(500, 500), true);
                }

                ImGui::EndChild();
            }

            ImGui::EndTable();
        }
    }

    ImGui::End();

    // Draw dialogs
    DrawCreateCategoryDialog();
    DrawLoadCategoryDialog();
}

void CategoryPanel::SelectCategory(int _idx)
{
    m_selectedCategory = _idx;
    m_textEditor->ClearText();

    auto l_category = m_editor.lock()
                      ->GetEngineContents().core
                      ->GetLuaContext()
                      ->GetCategory(m_availableCategories[m_selectedCategory])
                      .lock();

    m_selectedCategoryOrigin = l_category->GetOriginFile().lock();

    if(m_selectedCategoryOrigin != nullptr)
    {
        m_textEditor->SetText(m_selectedCategoryOrigin->GetCode());
    }
}

void CategoryPanel::DrawCreateCategoryDialog()
{
    if (m_showCreateDialog)
    {
        ImGui::OpenPopup("Create New Category");
    }

    if (ImGui::BeginPopupModal("Create New Category", &m_showCreateDialog, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Category Name:");
        ImGui::InputText("CategoryName", &m_newCategoryName[0], m_newCategoryName.size());

        ImGui::Separator();

        if (ImGui::Button("Create", ImVec2(120, 0)))
        {
            if (m_newCategoryName.size() > 0)
            {
                CreateNewCategory(m_newCategoryName);
                m_newCategoryName.clear();
                m_showCreateDialog = false;
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            m_showCreateDialog = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void CategoryPanel::DrawLoadCategoryDialog()
{
    if (!m_showLoadDialog)
        return;

    IGFD::FileDialogConfig config;
    config.path = ".";
    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".lua", config);


    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string l_filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string l_filePathDir = ImGuiFileDialog::Instance()->GetCurrentPath();
            LoadCategoryFromFile(l_filePathName);
        }


        ImGuiFileDialog::Instance()->Close();
        m_showLoadDialog = false;
    }
}

void CategoryPanel::CreateNewCategory(std::string _name)
{
    // Implement category creation logic
    RE::Log::Message("Creating new category: " + _name);
}

void CategoryPanel::LoadCategoryFromFile(std::string _path)
{
    RE::EngineContents l_engineContents = m_editor.lock()->GetEngineContents();
    auto l_luaContext = l_engineContents.core->GetLuaContext();

    auto l_categoryFile = l_engineContents.resources->Load<RE::Asset::LuaFile>(_path);
    auto l_categoryWPtr = l_luaContext->CreateCategory(l_categoryFile);
    
    m_availableCategories.push_back(l_categoryWPtr.lock()->GetName());
    RE::Log::Message("Category loaded from: " + _path);
}

void CategoryPanel::AssignCategoryToEntity(int _entityId, std::string _categoryName)
{
    auto l_scene = m_editor.lock()->GetScene();
    auto l_luaContext = RE::Core::LuaContext::Instance().lock();
    auto l_category = l_luaContext->GetCategory(_categoryName);
    
    l_scene->AddEntityToCategory(_entityId, l_category.lock()->GetSignature());
    RE::Log::Message("Category '" + _categoryName + "' assigned to entity " + std::to_string(_entityId));
}

bool CategoryPanel::IsDialogOpen()
{
    return m_showCreateDialog;
}

void CategoryPanel::SaveCategoryToFile()
{
    if (m_selectedCategoryOrigin != nullptr)
    {
        // Get the text from the editor
        std::string l_newCode = m_textEditor->GetText();
        
        // Save to the file
        m_selectedCategoryOrigin->SetCode(l_newCode);
        m_selectedCategoryOrigin->Save();
        
        // Clear the reload flag
        m_selectedCategoryOrigin->Reload();

        auto l_newCategory = m_selectedCategoryOrigin->GetCategory().lock();
        std::bitset<1024> l_catSignature = l_newCategory->GetSignature();

        auto l_scene = m_editor.lock()->GetScene();
        auto l_registry = l_scene->GetRegistry();

        // Remove category to delete all old data and readd category
        std::vector<int> l_entitiesInCategory = l_registry->GetEntitiesWith(l_catSignature);
        l_scene->RemoveCategory(l_catSignature);

        for(int l_entity : l_entitiesInCategory)
        {
            l_scene->AddEntityToCategory(l_entity, l_catSignature);
        }
        
        RE::Log::Message("Category saved to file: " + m_selectedCategoryOrigin->GetPath());
    }
}