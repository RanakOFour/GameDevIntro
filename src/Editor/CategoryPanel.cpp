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
, m_selectedCategory(-1)
, m_selectedCategoryOrigin()
, m_loadedCategories()
{
    RefreshCategoryList();
}

CategoryPanel::~CategoryPanel()
{

}

void CategoryPanel::RefreshCategoryList()
{
    m_loadedCategories.clear();

    auto l_context = m_editor.lock()->GetEngineContents()
                               .core->GetLuaContext();
    
    std::stringstream l_catNames(l_context->GetCategoryNames());
    std::string l_segment;

    while(std::getline(l_catNames, l_segment, ';'))
    {
        m_loadedCategories.push_back(l_segment);
    }

    std::sort(m_loadedCategories.begin(), m_loadedCategories.end());
}

void CategoryPanel::Draw()
{
    if (!m_showPanel) return;

    RefreshCategoryList();
    
    if (ImGui::Begin("Categories", &m_showPanel))
    {
        float buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
        // Create and Load buttons
        if (ImGui::Button("Create", ImVec2(buttonWidth, 0)))
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
        ImGui::InputTextWithHint("##CategoryFilter", "Search categories...", &m_filterString);

        ImGui::Separator();

        // Category list
        if (ImGui::BeginChild("CategoryList", ImVec2(0, 0), true))
        {
            for (int i = 0; i < m_loadedCategories.size(); i++)
            {
                auto& l_category = m_loadedCategories[i];

                if (m_filterString.size() > 0)
                {
                    std::string filter(m_filterString);
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
        }

        ImGui::EndChild();
        ImGui::End();
    }

    // Draw dialogs
    DrawCreateCategoryDialog();
    DrawLoadCategoryDialog();
}

void CategoryPanel::DrawAsChild(ImGuiChildFlags _flags)
{
    if (!m_showPanel) return;

    RefreshCategoryList();
    
    if (ImGui::BeginChild("Categories", ImVec2(0, 0), _flags))
    {
        float buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
        // Create and Load buttons
        if (ImGui::Button("Create", ImVec2(buttonWidth, 0)))
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
        ImGui::InputTextWithHint("##CategoryFilter", "Search categories...", &m_filterString);

        ImGui::Separator();

        // Category list
        if (ImGui::BeginChild("CategoryList", ImVec2(0, 0), true))
        {
            for (int i = 0; i < m_loadedCategories.size(); i++)
            {
                auto& l_category = m_loadedCategories[i];

                if (m_filterString.size() > 0)
                {
                    std::string filter(m_filterString);
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
        }

        ImGui::EndChild();
        ImGui::EndChild();
    }

    // Draw dialogs
    DrawCreateCategoryDialog();
    DrawLoadCategoryDialog();
}

void CategoryPanel::SelectCategory(int _idx)
{
    m_selectedCategory = _idx;
    auto l_category = m_editor.lock()
                      ->GetEngineContents().core
                      ->GetLuaContext()
                      ->GetCategory(m_loadedCategories[m_selectedCategory])
                      .lock();

    m_selectedCategoryOrigin = l_category->GetOriginFile().lock();
}

std::string CategoryPanel::GetCategoryAt(int _idx)
{
    assert(_idx < m_loadedCategories.size());

    return m_loadedCategories[_idx];
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

void CategoryPanel::CreateNewCategory(const std::string _name)
{
    // Implement category creation logic
    RE::Log::Message("Creating new category: " + _name);
}

void CategoryPanel::LoadCategoryFromFile(const std::string _path)
{
    RE::EngineContents l_engineContents = m_editor.lock()->GetEngineContents();
    auto l_luaContext = l_engineContents.core->GetLuaContext();

    auto l_categoryFile = l_engineContents.resources->Load<RE::Asset::LuaFile>(_path);
    auto l_categoryWPtr = l_luaContext->CreateCategory(l_categoryFile);
    
    m_loadedCategories.push_back(l_categoryWPtr.lock()->GetName());
    RE::Log::Message("Category loaded from: " + _path);
}

void CategoryPanel::AssignCategoryToEntity(const int _entityId, const std::string _categoryName)
{
    auto l_scene = m_editor.lock()->GetEngineContents().core->GetScene().lock();
    
    l_scene->AddToCategory(_entityId, _categoryName);
    RE::Log::Message("Category '" + _categoryName + "' assigned to entity " + std::to_string(_entityId));
}

std::weak_ptr<RE::Asset::LuaFile> CategoryPanel::GetSelectedFile()
{
    return m_selectedCategoryOrigin;
}

bool CategoryPanel::IsDialogOpen()
{
    return m_showCreateDialog;
}