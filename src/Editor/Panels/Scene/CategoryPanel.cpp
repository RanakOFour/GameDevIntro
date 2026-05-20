#include "Editor/Panels/Scene/CategoryPanel.h"
#include "Editor/Core/Editor.h"
#include "Editor/Scene/BuiltIn/BuiltinCategories.h"

#include <filesystem>
#include <fstream>

#include "imgui/imgui.h"

// String compatible functions for ImGui
#include "imgui/misc/cpp/imgui_stdlib.h"

#define USE_STD_FILESYSTEM 1
#include "imguiFileDialog/ImGuiFileDialog.h"

#include <algorithm>
#include <sstream>
#include <string>

CategoryPanel::CategoryPanel(Editor& _editor)
: Panel("Categories", _editor)
, m_showCreateDialog(false)
, m_showLoadDialog(false)
, m_showBuiltins(false)
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

    auto l_context = m_editor.GetEngineContents()
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
    if (m_needsRefresh)
    {
        RefreshCategoryList();
        m_needsRefresh = false;
    }
    
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

    // Show built-ins toggle
    ImGui::SameLine();
    ImGui::Checkbox("Built-in", &m_showBuiltins);

    ImGui::Separator();

    // Category list
    if (ImGui::BeginChild("CategoryList", ImVec2(0, 0), true))
    {
        for (int i = 0; i < (int)m_loadedCategories.size(); i++)
        {
            auto& l_category = m_loadedCategories[i];

            bool l_isBuiltin = BuiltinCategories::IsBuiltin(l_category);

            // Hide built-ins unless checkbox is set
            if (l_isBuiltin && !m_showBuiltins)
                continue;

            if (!m_filterString.empty())
            {
                if (l_category.find(m_filterString) == std::string::npos)
                    continue;
            }

            if (l_isBuiltin)
            {
                // Built-in: greyed out, not selectable
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.55f, 0.55f, 1.0f));
                ImGui::TextUnformatted((l_category + "  [built-in]").c_str());
                ImGui::PopStyleColor();
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Read-only built-in category");
            }
            else
            {
                bool l_selected = (m_selectedCategory == i);
                if (ImGui::Selectable(l_category.c_str(), l_selected))
                    SelectCategory(i);
            }
        }

        ImGui::EndChild();
    }

    // Draw dialogs
    DrawCreateCategoryDialog();
}

void CategoryPanel::SelectCategory(int _idx)
{
    m_selectedCategory = _idx;
    auto l_category = m_editor.GetEngineContents().core
                      ->GetLuaContext()
                      ->GetCategory(m_loadedCategories[m_selectedCategory])
                      .lock();

    if (l_category)
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
        ImGui::InputText("CategoryName", &m_newCategoryName);

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

    auto& l_editor = m_editor;
    std::string l_defaultPath = l_editor.GetProject().IsOpen()
                              ? l_editor.GetProject().GetCategoriesDir()
                              : ".";

    IGFD::FileDialogConfig config;
    config.path = l_defaultPath;
    ImGuiFileDialog::Instance()->OpenDialog("CategoryFileDlgKey", "Choose File", ".lua", config);


    if (ImGuiFileDialog::Instance()->Display("CategoryFileDlgKey", ImGuiWindowFlags_None, ImVec2(600, 400)))
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
    Editor& l_editor = m_editor;
    if (!l_editor.GetProject().IsOpen())
    {
        RE::Log::Warning("No project open — cannot create category.");
        return;
    }

    std::filesystem::path l_dir  = l_editor.GetProject().GetCategoriesDir();
    std::filesystem::path l_path = l_dir / (_name + ".lua");

    std::filesystem::create_directories(l_dir);

    if (std::filesystem::exists(l_path))
    {
        RE::Log::Warning("Category already exists: " + l_path.string());
        return;
    }

    std::ofstream l_file(l_path);
    if (!l_file.is_open())
    {
        RE::Log::Error("Failed to create file: " + l_path.string());
        return;
    }

    l_file << "return Category {\n"
           << "    -- Add your fields here\n"
           << "}\n";
    l_file.close();

    RE::Log::Message("Category created: " + l_path.string());

    LoadCategoryFromFile(l_path.string());
}

void CategoryPanel::LoadCategoryFromFile(const std::string _path)
{
    RE::EngineContents l_engineContents = m_editor.GetEngineContents();
    auto l_luaContext = l_engineContents.core->GetLuaContext();

    auto l_categoryFile = l_engineContents.resources->Load<RE::Asset::LuaFile>(_path);
    auto l_categoryWPtr = l_luaContext->CreateCategory(l_categoryFile);
    auto l_categoryPtr = l_categoryWPtr.lock();
    if (!l_categoryPtr)
    {
        RE::Log::Error("Failed to create category from: " + _path);
        return;
    }
    std::string l_newName = l_categoryPtr->GetName();
    m_loadedCategories.push_back(l_newName);

    RefreshCategoryList();

    auto it = std::find(m_loadedCategories.begin(), m_loadedCategories.end(), l_newName);
    if (it != m_loadedCategories.end())
        SelectCategory(static_cast<int>(std::distance(m_loadedCategories.begin(), it)));

    m_editor.SaveProjectInfo();
    RE::Log::Message("Category loaded from: " + _path);
}

void CategoryPanel::AssignCategoryToEntity(const int _entityId, const std::string _categoryName)
{
    auto l_scene = m_editor.GetEngineContents().core->GetScene().lock();
    
    l_scene->AddToCategory(_entityId, _categoryName);
    m_editor.FireTutorialEvent("category_added");
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