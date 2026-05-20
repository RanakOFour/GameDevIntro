#include "Editor/Panels/Scene/RulesPanel.h"
#include "Editor/Core/Editor.h"
#include "Editor/Scene/BuiltIn/BuiltinRules.h"
#include "Editor/Tabs/SceneEditTab.h"

#include <filesystem>
#include <fstream>

#include "imgui/imgui.h"

// String compatible functions for ImGui
#include "imgui/misc/cpp/imgui_stdlib.h"

#define USE_STD_FILESYSTEM 1
#include "imguiFileDialog/ImGuiFileDialog.h"

#include <algorithm>

RulesPanel::RulesPanel(Editor& _editor)
: Panel("Rules", _editor)
, m_showCreateDialog(false)
, m_showLoadDialog(false)
, m_showBuiltins(false)
, m_newRuleName("")
, m_loadedRules()
, m_activeRules()
, m_selectedRuleIndex(-1)
{
}

RulesPanel::~RulesPanel()
{
}

void RulesPanel::RefreshRuleList()
{
    m_activeRules.clear();
    m_loadedRules.clear();

    auto l_scene = m_editor.GetEngineContents().core->GetScene().lock();
    if (!l_scene) return;
    sol::table l_rulesTable = l_scene->GetSceneTable().raw_get<sol::table>("Rules");

    // Pull every rule currently in the scene (user + built-in). The Draw loop
    // filters built-ins via m_showBuiltins; the registry only tracks user rules
    // so it can't drive this list on its own.
    for (auto& l_pair : l_rulesTable.pairs())
    {
        if (l_pair.first.get_type() != sol::type::string) continue;
        m_loadedRules.push_back(l_pair.first.as<std::string>());
    }

    for (const auto& name : m_loadedRules)
    {
        auto l_rulePtr = l_rulesTable.raw_get<std::shared_ptr<RE::Core::Rule>>(name);
        if (l_rulePtr && l_rulePtr->GetActive())
            m_activeRules.push_back(name);
    }
}

void RulesPanel::Draw()
{
    if (m_needsRefresh)
    {
        RefreshRuleList();
        m_needsRefresh = false;
    }

    // Create and Load buttons
    if (ImGui::Button("Create", ImVec2((ImGui::GetContentRegionAvail().x - 5) * 0.5f, 0)))
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
    ImGui::InputTextWithHint("##RulesFilter", "Search rules...", &m_filterString);

    // Show built-ins toggle
    ImGui::SameLine();
    ImGui::Checkbox("Built-in", &m_showBuiltins);

    ImGui::Separator();
    
    sol::table l_sceneTable = m_editor.GetEngineContents().core->GetScene().lock()->GetSceneTable();

    // Active rules list
    if (ImGui::BeginChild("RulesList", ImVec2(0, 0), true))
    {
        for (int i = 0; i < m_loadedRules.size(); i++)
        {
            std::string l_ruleName = m_loadedRules[i];
            bool l_isBuiltin = BuiltinRules::IsBuiltin(l_ruleName);

            // Hide built-ins unless checkbox is set
            if (l_isBuiltin && !m_showBuiltins)
                continue;

            if (!m_filterString.empty())
            {
                if (l_ruleName.find(m_filterString) == std::string::npos)
                    continue;
            }

            sol::optional<std::shared_ptr<RE::Core::Rule>> l_rulePtrOpt = l_sceneTable.traverse_raw_get<sol::optional<std::shared_ptr<RE::Core::Rule>>>("Rules", l_ruleName);
            if (!l_rulePtrOpt.has_value()) continue;
            std::shared_ptr<RE::Core::Rule> l_rulePtr = *l_rulePtrOpt;

            ImGui::PushID(l_ruleName.c_str());

            bool l_active = l_rulePtr->GetActive();

            if (l_isBuiltin)
            {
                // Greyed label with [built-in] tag
                ImGui::PushStyleColor(ImGuiCol_Text, l_active
                    ? ImVec4(0.75f, 0.75f, 0.75f, 1.0f)
                    : ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
                ImGui::TextUnformatted((l_ruleName + "  [built-in]" + (l_active ? "" : " (Inactive)")).c_str());
                ImGui::PopStyleColor();

                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Read-only built-in rule. Toggle to enable/disable.");
            }
            else
            {
                std::string displayName = l_ruleName + (l_active ? "" : " (Inactive)");
                if (ImGui::Selectable(displayName.c_str(), m_selectedRuleIndex == i))
                {
                    SelectRule(i);
                }
            }

            // Toggle button — available for all rules including built-ins
            ImGui::SameLine(ImGui::GetWindowWidth() - 100);
            if (ImGui::Button("Toggle", ImVec2(0, 0)))
            {
                l_rulePtr->SetActive(!l_active);
                RE::Log::Message("Set " + l_ruleName + " to "
                                 + (l_rulePtr->GetActive() ? "active" : "not active") + "!");
            }

            ImGui::PopID();
        }
        ImGui::EndChild();
    }

    // Draw dialogs
    DrawCreateRuleDialog();
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
        ImGui::InputText("RuleName", &m_newRuleName);

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

    std::string l_defaultPath = m_editor.GetProject().IsOpen()
                              ? m_editor.GetProject().GetRulesDir()
                              : ".";

    IGFD::FileDialogConfig config;
    config.path = l_defaultPath;
    ImGuiFileDialog::Instance()->OpenDialog("RuleFileDlgKey", "Choose File", ".lua", config);


    if (ImGuiFileDialog::Instance()->Display("RuleFileDlgKey", ImGuiWindowFlags_None, ImVec2(600, 400)))
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
    if (!m_editor.GetProject().IsOpen())
    {
        RE::Log::Warning("No project open — cannot create rule.");
        return;
    }

    std::filesystem::path l_dir  = m_editor.GetProject().GetRulesDir();
    std::filesystem::path l_path = l_dir / (_name + ".lua");

    std::filesystem::create_directories(l_dir);

    if (std::filesystem::exists(l_path))
    {
        RE::Log::Warning("Rule already exists: " + l_path.string());
        return;
    }

    std::ofstream l_file(l_path);
    if (!l_file.is_open())
    {
        RE::Log::Error("Failed to create file: " + l_path.string());
        return;
    }

    l_file << "local " << _name << " = Rule {\n"
           << "    categories = { \"Transform\" },\n"
           << "}\n"
           << "\n"
           << "function " << _name << ":Update(_entityData)\n"
           << "    -- Add your logic here\n"
           << "end\n"
           << "\n"
           << "return " << _name << "\n";
    l_file.close();

    RE::Log::Message("Rule created: " + l_path.string());

    LoadRuleFromFile(l_path.string());
}

void RulesPanel::LoadRuleFromFile(const std::string _path)
{
    RE::EngineContents l_engineContents = m_editor.GetEngineContents();
    auto l_luaContext = l_engineContents.core->GetLuaContext();

    auto l_ruleFile = l_engineContents.resources->Load<RE::Asset::LuaFile>(_path);
    RE::Core::Rule l_newRule = l_luaContext->CreateRule(l_ruleFile);
    
    l_engineContents.core->GetScene().lock()->AddRule(l_newRule);
    m_editor.FireTutorialEvent("rule_added");

    // Register in the editor-level registry so the rule survives scene changes.
    m_editor.GetSceneEdit().RegisterRule(l_newRule.GetName(), _path);

    std::string l_newName = l_newRule.GetName();
    m_loadedRules.push_back(l_newName);

    RefreshRuleList();

    auto it = std::find(m_loadedRules.begin(), m_loadedRules.end(), l_newName);
    if (it != m_loadedRules.end())
        SelectRule(static_cast<int>(std::distance(m_loadedRules.begin(), it)));

    m_editor.SaveProjectInfo();
    RE::Log::Message("Rule loaded from: " + _path);
}

void RulesPanel::RemoveRule(const std::string _ruleName)
{
    auto it = std::find(m_loadedRules.begin(), m_loadedRules.end(), _ruleName);
    if (it != m_loadedRules.end())
    {
        m_loadedRules.erase(it);
        m_needsRefresh = true;
        RE::Log::Message("Rule removed: " + _ruleName);
    }
}

void RulesPanel::SelectRule(int _idx)
{
    m_selectedRuleIndex = _idx;

    sol::table l_sceneTable = m_editor.GetEngineContents().core->GetScene().lock()->GetSceneTable();
    auto l_rulePtr = l_sceneTable.traverse_raw_get<std::shared_ptr<RE::Core::Rule>>("Rules", m_loadedRules[_idx]);
    if (l_rulePtr)
        m_selectedRuleFile = l_rulePtr->GetOriginFile().lock();
}

std::weak_ptr<RE::Asset::LuaFile> RulesPanel::GetSelectedFile()
{
    return m_selectedRuleFile;
}

bool RulesPanel::IsDialogOpen()
{
    return m_showLoadDialog || m_showCreateDialog;
}