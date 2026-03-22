#include "Editor/CategoryPanel.h"
#include "Editor/Editor.h"

#include "imgui/imgui.h"

// String compatible functions for ImGui
#include "imgui/misc/cpp/imgui_stdlib.h"

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
    
    ImGui::SetNextWindowSize(ImVec2(800, 400));
    if (ImGui::Begin("Categories", &m_showPanel))
    {
        if(ImGui::BeginTable("Categories", 2, ImGuiTableFlags_BordersInnerV))
        {
            ImGui::TableSetupColumn("Categories", ImGuiTableColumnFlags_WidthFixed, 250);
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
                    m_loadCategoryFilePath = m_editor.lock()->GetEngineContents().io->OpenFileDialog();
                    LoadCategoryFromFile(m_loadCategoryFilePath);
                }

                ImGui::Separator();

                // Search/Filter
                ImGui::InputTextWithHint("CategoryFilter", "Search categories...", &m_selectedCategoryFilter[0], m_selectedCategoryFilter.size());

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
                ImGui::Text("Category: %s", m_availableCategories[m_selectedCategory].c_str());
                ImGui::Separator();

                if(m_selectedCategoryOrigin != nullptr)
                {
                    if(m_selectedCategoryOrigin->GetReloaded())
                    {
                        ImGui::Text("%s*", m_availableCategories[m_selectedCategory]);
                    }
                    else
                    {
                        ImGui::Text("%s", m_availableCategories[m_selectedCategory]);
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
    if (m_showLoadDialog)
    {
        ImGui::OpenPopup("Load Category");
    }

    if (ImGui::BeginPopupModal("Load Category", &m_showLoadDialog, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Category File Path:");

        ImGui::InputText("CategoryPath", &m_loadCategoryFilePath, ImGuiInputTextFlags_EnterReturnsTrue);

        ImGui::Separator();

        if (ImGui::Button("Load", ImVec2(120, 0)))
        {
            RE::Log::Message("Creating category from " + m_loadCategoryFilePath);
            LoadCategoryFromFile(m_loadCategoryFilePath);
            m_loadCategoryFilePath = "./resources/Categories/";
            m_showLoadDialog = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            m_loadCategoryFilePath = "./resources/Categories/";
            m_showLoadDialog = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void CategoryPanel::CreateNewCategory(const std::string& _name)
{
    // TODO: Implement category creation logic
    RE::Log::Message("Creating new category: " + _name);
}

void CategoryPanel::LoadCategoryFromFile(const std::string& _path)
{
    RE::EngineContents l_engineContents = m_editor.lock()->GetEngineContents();
    auto l_luaContext = l_engineContents.core->GetLuaContext();

    auto l_categoryFile = l_engineContents.resources->Load<RE::Asset::LuaFile>(_path);
    auto l_categoryWPtr = l_luaContext->CreateCategory(l_categoryFile);
    
    m_availableCategories.push_back(l_categoryWPtr.lock()->GetName());
    RE::Log::Message("Category loaded from: " + _path);
}

void CategoryPanel::AssignCategoryToEntity(int _entityId, const std::string& _categoryName)
{
    auto l_scene = m_editor.lock()->GetScene();
    auto l_luaContext = RE::Core::LuaContext::Instance().lock();
    auto l_category = l_luaContext->GetCategory(_categoryName);
    
    l_scene->AddEntityToCategory(_entityId, l_category.lock()->GetSignature());
    RE::Log::Message("Category '" + _categoryName + "' assigned to entity " + std::to_string(_entityId));
}

bool CategoryPanel::IsDialogOpen()
{
    return m_showCreateDialog || m_showLoadDialog;
}
