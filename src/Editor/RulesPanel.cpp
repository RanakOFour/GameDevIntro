#include "Editor/RulesPanel.h"
#include "Editor/Editor.h"
#include "imgui/imgui.h"
#include <algorithm>

RulesPanel::RulesPanel(std::weak_ptr<Editor> _editor)
: m_editor(_editor)
, m_showPanel(true)
, m_showCreateDialog(false)
, m_showLoadDialog(false)
, m_newRuleName("")
, m_selectedRuleFilter("")
{
    RefreshRuleList();
}

RulesPanel::~RulesPanel()
{
}

void RulesPanel::RefreshRuleList()
{
    m_activeRules.clear();
    m_availableRuleFiles.clear();
    
    auto editor = m_editor.lock();
    if (editor)
    {
        auto scene = editor->GetScene();
        if (scene)
        {
            auto registry = scene->GetRegistry();
            // TODO: Get all active rules from scene
        }
    }
}

void RulesPanel::Draw()
{
    if (!m_showPanel) return;

    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Rules##panel", &m_showPanel))
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
        ImGui::InputTextWithHint("##RuleFilter", "Search rules...", m_selectedRuleFilter.data(), m_selectedRuleFilter.size());

        ImGui::Separator();

        // Active rules list
        if (ImGui::BeginChild("RulesList", ImVec2(0, 0), true))
        {
            for (const auto& rule : m_activeRules)
            {
                if (m_selectedRuleFilter.size() > 0)
                {
                    std::string filter(m_selectedRuleFilter);
                    if (rule.find(filter) == std::string::npos)
                    {
                        continue;
                    }
                }

                ImGui::PushID(rule.c_str());
                
                if (ImGui::Selectable(rule.c_str(), false))
                {
                    // Handle rule selection
                }

                // Remove button
                ImGui::SameLine(ImGui::GetWindowWidth() - 30);
                if (ImGui::Button("X##remove", ImVec2(25, 0)))
                {
                    RemoveRule(rule);
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
        ImGui::InputText("##RuleName", &m_newRuleName[0], m_newRuleName.size());

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
    if (m_showLoadDialog)
    {
        ImGui::OpenPopup("Load Rule");
    }

    if (ImGui::BeginPopupModal("Load Rule", &m_showLoadDialog, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static char filePath[512] = "./resources/Rules/";
        ImGui::Text("Rule File Path:");
        ImGui::InputText("##RulePath", filePath, IM_ARRAYSIZE(filePath));

        ImGui::Separator();

        if (ImGui::Button("Load", ImVec2(120, 0)))
        {
            LoadRuleFromFile(filePath);
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

void RulesPanel::CreateNewRule(const std::string& _name)
{
    // TODO: Implement rule creation with template
    RE::Log::Message("Creating new rule: " + _name);
}

void RulesPanel::LoadRuleFromFile(const std::string& _path)
{
    auto editor = m_editor.lock();
    if (editor)
    {
        auto& engineContents = editor->GetEngineContents();
        auto scene = editor->GetScene();
        auto luaContext = RE::Core::LuaContext::Instance().lock();

        try
        {
            auto ruleFile = engineContents.resources->Load<RE::Asset::LuaFile>(_path);
            RE::Core::Rule newRule = luaContext->RunScript<RE::Core::Rule>(ruleFile);
            
            scene->AddRule(newRule);
            m_activeRules.push_back(newRule.GetName());
            
            RE::Log::Message("Rule loaded from: " + _path);
        }
        catch (const std::exception& e)
        {
            RE::Log::Error("Failed to load rule: " + std::string(e.what()));
        }
    }
}

void RulesPanel::RemoveRule(const std::string& _ruleName)
{
    auto it = std::find(m_activeRules.begin(), m_activeRules.end(), _ruleName);
    if (it != m_activeRules.end())
    {
        m_activeRules.erase(it);
        RE::Log::Message("Rule removed: " + _ruleName);
    }
}
