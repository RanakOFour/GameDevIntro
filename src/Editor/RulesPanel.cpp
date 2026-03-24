#include "Editor/RulesPanel.h"
#include "Editor/Editor.h"

#include "imgui/imgui.h"

// String compatible functions for ImGui
#include "imgui/misc/cpp/imgui_stdlib.h"

#define USE_STD_FILESYSTEM 1
#include "imguiFileDialog/ImGuiFileDialog.h"

#include <algorithm>

RulesPanel::RulesPanel(std::weak_ptr<Editor> _editor)
: Panel(_editor)
, m_showCreateDialog(false)
, m_showLoadDialog(false)
, m_newRuleName("")
, m_loadedRules()
, m_activeRules()
{
    RefreshRuleList();
}

RulesPanel::~RulesPanel()
{
}

void RulesPanel::RefreshRuleList()
{
    m_activeRules.clear();
    m_loadedRules.clear();
    
    auto l_scene = m_editor.lock()->GetEngineContents().core->GetScene().lock();

    // Get active rules from the scene
    sol::table l_sceneTable = l_scene->GetSceneTable();
    sol::table l_rulesTable = l_sceneTable["Rules"];
    auto l_rulesPairs = l_rulesTable.pairs();

    for(auto& l_pair : l_rulesPairs)
    {
        std::string l_ruleName = l_pair.first.as<std::string>();
        m_loadedRules.push_back(l_ruleName);

        std::shared_ptr<RE::Core::Rule> l_rulePtr = l_rulesTable.raw_get<std::shared_ptr<RE::Core::Rule>>(l_ruleName);
        if(l_rulePtr->GetActive())
        {
            m_activeRules.push_back(l_ruleName);
        }
    }
}

void RulesPanel::Draw()
{
    if (!m_showPanel) return;

    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Rules", &m_showPanel))
    {
        ImGui::Text("Active Rules:");
        ImGui::Separator();

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
        ImGui::InputTextWithHint("RuleFilter", "Search rules...", &m_filterString);

        ImGui::Separator();

        // Active rules list
        if (ImGui::BeginChild("RulesList", ImVec2(0, 0), true))
        {
            for (auto& l_ruleName : m_activeRules)
            {
                if (m_filterString.size() > 0)
                {
                    std::string filter(m_filterString);
                    if (l_ruleName.find(filter) == std::string::npos)
                    {
                        continue;
                    }
                }

                ImGui::PushID(l_ruleName.c_str());
                
                ImGui::Text(l_ruleName.c_str());

                // Remove button
                ImGui::SameLine(ImGui::GetWindowWidth() - 100);
                if (ImGui::Button("Toggle", ImVec2(0, 0)))
                {
                    // Set rule to not active
                    sol::table l_sceneTable = m_editor.lock()->GetEngineContents().core->GetScene().lock()->GetSceneTable();
                    std::shared_ptr<RE::Core::Rule> l_rulePtr = l_sceneTable.traverse_raw_get<std::shared_ptr<RE::Core::Rule>>("Rules", l_ruleName);
                    l_rulePtr->SetActive(!l_rulePtr->GetActive());
                    
                    std::string l_logMessage = "Set " + l_ruleName + " to ";

                    if(!l_rulePtr->GetActive())
                    {
                        l_logMessage += "not ";
                    }

                    RE::Log::Message(l_logMessage + "active!");
                }

                ImGui::PopID();
            }
            ImGui::EndChild();
        }
    }
    ImGui::End();

    // Draw dialogs
    DrawCreateRuleDialog();
    DrawLoadRuleDialog();
}

void RulesPanel::DrawCreateRuleDialog()
{
    if (m_showCreateDialog)
    {
        ImGui::OpenPopup("Create New Rule");
    }

    if (ImGui::BeginPopupModal("Create New Rule", &m_showCreateDialog, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Rule Name:");
        ImGui::InputText("RuleName", &m_newRuleName[0], m_newRuleName.size());

        ImGui::Separator();

        ImGui::Text("Template:");
        static int templateIndex = 0;
        ImGui::RadioButton("Empty", &templateIndex, 0);
        ImGui::RadioButton("Transform Update", &templateIndex, 1);
        ImGui::RadioButton("Rendering", &templateIndex, 2);

        ImGui::Separator();

        if (ImGui::Button("Create", ImVec2(120, 0)))
        {
            if (m_newRuleName.size() > 0)
            {
                CreateNewRule(m_newRuleName);
                m_newRuleName.clear();
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

void RulesPanel::DrawLoadRuleDialog()
{
    if (!m_showLoadDialog)
    {
        return;
    }

    IGFD::FileDialogConfig config;
    config.path = ".";
    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".lua", config);


    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string l_filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            LoadRuleFromFile(l_filePathName);
            RefreshRuleList();
        }


        ImGuiFileDialog::Instance()->Close();
        m_showLoadDialog = false;
    }
}

void RulesPanel::CreateNewRule(const std::string _name)
{
    // TODO: Implement rule creation with template
    RE::Log::Message("Creating new rule: " + _name);
}

void RulesPanel::LoadRuleFromFile(const std::string _path)
{
    RE::EngineContents l_engineContents = m_editor.lock()->GetEngineContents();
    auto l_luaContext = l_engineContents.core->GetLuaContext();

    auto l_ruleFile = l_engineContents.resources->Load<RE::Asset::LuaFile>(_path);
    RE::Core::Rule l_newRule = l_luaContext->RunScript<RE::Core::Rule>(l_ruleFile);
    
    l_engineContents.core->GetScene().lock()->AddRule(l_newRule);

    m_loadedRules.push_back(l_newRule.GetName());
    RE::Log::Message("Category loaded from: " + _path);
}

void RulesPanel::RemoveRule(const std::string _ruleName)
{
    auto it = std::find(m_loadedRules.begin(), m_loadedRules.end(), _ruleName);
    if (it != m_loadedRules.end())
    {
        m_loadedRules.erase(it);
        RE::Log::Message("Rule removed: " + _ruleName);
    }
}

bool RulesPanel::IsDialogOpen()
{
    return m_showLoadDialog || m_showCreateDialog;
}