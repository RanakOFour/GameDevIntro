#include "Editor/RulesPanel.h"
#include "Editor/Editor.h"
#include "Editor/BuiltinRules.h"
#include "Editor/SceneEditTab.h"

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
{
}

RulesPanel::~RulesPanel()
{
}

void RulesPanel::RefreshRuleList()
{
    m_activeRules.clear();
    m_loadedRules.clear();

    // Collect built-in rule names from the scene (always present).
    auto l_scene = m_editor.GetEngineContents().core->GetScene().lock();
    sol::table l_rulesTable = l_scene->GetSceneTable().raw_get<sol::table>("Rules");

    // Built-in names
    for (const auto& entry : BuiltinRules::GetEntries())
        m_loadedRules.push_back(entry.name);

    // User rules from the persistent registry
    for (const auto& record : m_editor.GetSceneEdit().GetRuleRegistry())
        m_loadedRules.push_back(record.name);

    // Determine active status from the scene table
    for (const auto& name : m_loadedRules)
    {
        auto l_obj = l_rulesTable.raw_get<sol::object>(name);
        if (!l_obj.valid() || l_obj.get_type() == sol::type::nil) continue;

        auto l_rulePtr = l_rulesTable.raw_get<std::shared_ptr<RE::Core::Rule>>(name);
        if (l_rulePtr && l_rulePtr->GetActive())
            m_activeRules.push_back(name);
    }
}

void RulesPanel::Draw()
{
    RefreshRuleList();

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
        for (auto& l_ruleName : m_loadedRules)
        {
            bool l_isBuiltin = BuiltinRules::IsBuiltin(l_ruleName);

            // Hide built-ins unless checkbox is set
            if (l_isBuiltin && !m_showBuiltins)
                continue;

            if (!m_filterString.empty())
            {
                if (l_ruleName.find(m_filterString) == std::string::npos)
                    continue;
            }

            ImGui::PushID(l_ruleName.c_str());

            std::shared_ptr<RE::Core::Rule> l_rulePtr =
                l_sceneTable.traverse_raw_get<std::shared_ptr<RE::Core::Rule>>("Rules", l_ruleName);

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
                ImGui::Text("%s", l_active ? l_ruleName.c_str()
                                           : (l_ruleName + " (Inactive)").c_str());
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

    std::string l_defaultPath = m_editor.GetProject().IsOpen()
                              ? m_editor.GetProject().GetRulesDir()
                              : ".";

    IGFD::FileDialogConfig config;
    config.path = l_defaultPath;
    ImGuiFileDialog::Instance()->OpenDialog("RuleFileDlgKey", "Choose File", ".lua", config);


    if (ImGuiFileDialog::Instance()->Display("RuleFileDlgKey"))
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
    RE::EngineContents l_engineContents = m_editor.GetEngineContents();
    auto l_luaContext = l_engineContents.core->GetLuaContext();

    auto l_ruleFile = l_engineContents.resources->Load<RE::Asset::LuaFile>(_path);
    RE::Core::Rule l_newRule = l_luaContext->CreateRule(l_ruleFile);
    
    l_engineContents.core->GetScene().lock()->AddRule(l_newRule);

    // Register in the editor-level registry so the rule survives scene changes.
    m_editor.GetSceneEdit().RegisterRule(l_newRule.GetName(), _path);

    m_loadedRules.push_back(l_newRule.GetName());
    RE::Log::Message("Rule loaded from: " + _path);
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