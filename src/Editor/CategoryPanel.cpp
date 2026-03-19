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
{
    RefreshCategoryList();
}

CategoryPanel::~CategoryPanel()
{
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
    
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Categories", &m_showPanel))
    {
        // Create and Load buttons
        if (ImGui::Button("+ Create", ImVec2((ImGui::GetContentRegionAvail().x - 5) * 0.5f, 0)))
        {
            m_showCreateDialog = true;
        }

        ImGui::SameLine();

        if (ImGui::Button("Load", ImVec2((ImGui::GetContentRegionAvail().x), 0)))
        {
            m_showLoadDialog = true;
        }

        ImGui::Separator();

        // Search/Filter
        ImGui::InputTextWithHint("CategoryFilter", "Search categories...", &m_selectedCategoryFilter[0], m_selectedCategoryFilter.size());

        ImGui::Separator();

        // Category list
        if (ImGui::BeginChild("CategoryList", ImVec2(0, 0), true))
        {
            for (auto& l_category : m_availableCategories)
            {
                if (m_selectedCategoryFilter.size() > 0)
                {
                    std::string filter(m_selectedCategoryFilter);
                    if (l_category.find(filter) == std::string::npos)
                    {
                        continue;
                    }
                }

                ImGui::Text(l_category.c_str());
            }
            
            ImGui::EndChild();
        }
    }

    ImGui::End();

    // Draw dialogs
    DrawCreateCategoryDialog();
    DrawLoadCategoryDialog();
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
