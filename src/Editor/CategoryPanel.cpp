#include "Editor/CategoryPanel.h"
#include "Editor/Editor.h"
#include "imgui/imgui.h"

#include <algorithm>

CategoryPanel::CategoryPanel(std::weak_ptr<Editor> _editor)
: m_editor(_editor)
, m_showPanel(true)
, m_showCreateDialog(false)
, m_showLoadDialog(false)
, m_newCategoryName("")
, m_selectedCategoryFilter("")
{
    RefreshCategoryList();
}

CategoryPanel::~CategoryPanel()
{
}

void CategoryPanel::RefreshCategoryList()
{
    m_availableCategories.clear();
    // TODO: Get all available categories from the core manager
    // This depends on the API to access all loaded categories
}

void CategoryPanel::Draw()
{
    if (!m_showPanel) return;

    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Categories##panel", &m_showPanel))
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
        ImGui::InputTextWithHint("##CategoryFilter", "Search categories...", &m_selectedCategoryFilter[0], m_selectedCategoryFilter.size());

        ImGui::Separator();

        // Category list
        if (ImGui::BeginChild("CategoryList", ImVec2(0, 0), true))
        {
            for (const auto& category : m_availableCategories)
            {
                if (m_selectedCategoryFilter.size() > 0)
                {
                    std::string filter(m_selectedCategoryFilter);
                    if (category.find(filter) == std::string::npos)
                    {
                        continue;
                    }
                }

                if (ImGui::Selectable(category.c_str(), false))
                {
                    // Assignment logic here
                    auto editor = m_editor.lock();
                    if (editor)
                    {
                        int selectedEntity = editor->GetSelectedEntityId();
                        if (selectedEntity >= 0)
                        {
                            AssignCategoryToEntity(selectedEntity, category);
                        }
                    }
                }
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
        ImGui::InputText("##CategoryName", &m_newCategoryName[0], m_newCategoryName.size());

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
        static char filePath[512] = "./resources/Categories/";
        ImGui::Text("Category File Path:");
        ImGui::InputText("##CategoryPath", filePath, IM_ARRAYSIZE(filePath));

        ImGui::Separator();

        if (ImGui::Button("Load", ImVec2(120, 0)))
        {
            LoadCategoryFromFile(filePath);
            m_showLoadDialog = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
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
    auto editor = m_editor.lock();
    if (editor)
    {
        auto& engineContents = editor->GetEngineContents();
        auto luaContext = RE::Core::LuaContext::Instance().lock();

        try
        {
            auto categoryFile = engineContents.resources->Load<RE::Asset::LuaFile>(_path);
            auto category = luaContext->CreateCategory(categoryFile);
            
            m_availableCategories.push_back(_path);
            RE::Log::Message("Category loaded from: " + _path);
        }
        catch (const std::exception& e)
        {
            RE::Log::Error("Failed to load category: " + std::string(e.what()));
        }
    }
}

void CategoryPanel::AssignCategoryToEntity(int _entityId, const std::string& _categoryName)
{
    auto editor = m_editor.lock();
    if (editor)
    {
        auto scene = editor->GetScene();
        if (scene)
        {
            auto luaContext = RE::Core::LuaContext::Instance().lock();
            auto category = luaContext->GetCategory(_categoryName);
            
            if (!category.expired())
            {
                scene->AddEntityToCategory(_entityId, category.lock()->GetSignature());
                RE::Log::Message("Category '" + _categoryName + "' assigned to entity " + std::to_string(_entityId));
            }
        }
    }
}
