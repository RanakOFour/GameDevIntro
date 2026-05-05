#include "Editor/Panels/UI/TutorialPanel.h"
#include "Editor/Core/Editor.h"
#include "Editor/Core/StateRegistry.h"

#include "RanakEngine/Assets.h"
#include "RanakEngine/Core.h"
#include "RanakEngine/Asset/Texture.h"
#include "RanakEngine/Asset/LuaFile.h"

#include "imgui/imgui.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

#include <algorithm>
#include <sstream>

TutorialPanel::TutorialPanel(Editor& _editor)
: Panel("Tutorial", _editor)
{
    m_showPanel = false;
}

void TutorialPanel::LoadTutorial(const std::string& _path)
{
    m_steps.clear();
    m_imageCache.clear();
    m_currentStep = 0;
    m_tutorialTitle = "";

    auto l_file = RE::Asset::Load<RE::Asset::LuaFile>(_path);
    if (l_file.expired())
    {
        RE::Log::Error("TutorialPanel: could not load tutorial file: " + _path);
        return;
    }

    sol::table l_table = RE::Core::LuaContext::Instance().lock()->RunScript<sol::table>(l_file);
    if (!l_table.valid())
    {
        RE::Log::Error("TutorialPanel: tutorial script did not return a table: " + _path);
        return;
    }

    // Optional title for the whole tutorial
    sol::optional<std::string> l_tutTitle = l_table.get<sol::optional<std::string>>("title");
    if (l_tutTitle.has_value())
        m_tutorialTitle = *l_tutTitle;

    sol::optional<sol::table> l_stepsTable = l_table.get<sol::optional<sol::table>>("steps");
    sol::table l_source = l_stepsTable.has_value() ? *l_stepsTable : l_table;

    for (auto& l_pair : l_source)
    {
        if (l_pair.second.get_type() != sol::type::table)
            continue;

        sol::table l_stepTable = l_pair.second.as<sol::table>();
        TutorialStep l_step;
        l_step.title = l_stepTable.get_or("title",        std::string{});
        l_step.body = l_stepTable.get_or("body",          std::string{});
        l_step.imagePath = l_stepTable.get_or("image",    std::string{});
        l_step.highlightKey = l_stepTable.get_or("highlight", std::string{});
        l_step.event = l_stepTable.get_or("event",        std::string{});
        l_step.forceState = l_stepTable.get_or("force_state", std::string{});
        l_step.waitState  = l_stepTable.get_or("wait_state",  std::string{});
        sol::optional<sol::table> l_clTable = l_stepTable.get<sol::optional<sol::table>>("checklist");
        if (l_clTable.has_value())
        {
            for (auto& l_clPair : *l_clTable)
            {
                if (l_clPair.second.get_type() != sol::type::table) continue;
                sol::table l_clItem = l_clPair.second.as<sol::table>();
                ChecklistItem l_ci;
                l_ci.label = l_clItem.get_or("label", std::string{});
                l_ci.event = l_clItem.get_or("event", std::string{});
                l_step.checklist.push_back(l_ci);
            }
        }
        m_steps.push_back(l_step);

        // Pre-load any images
        if (!l_step.imagePath.empty() && m_imageCache.find(l_step.imagePath) == m_imageCache.end())
        {
            auto l_tex = RE::Asset::Load<RE::Asset::Texture>(l_step.imagePath).lock();
            if (l_tex)
            {
                m_imageCache[l_step.imagePath] = l_tex;
            }
        }
    }

    if (!m_steps.empty())
    {
        m_showPanel = true;
        m_lastAppliedForceStep = -1;
        m_waitSatisfiedAtEntry = false;
    }

    RE::Log::Message("TutorialPanel: loaded " + std::to_string(m_steps.size()) + " steps from " + _path);
}

void TutorialPanel::RegisterRegion(const std::string& _key, ImRect _rect)
{
    m_regions[_key] = _rect;
}

bool TutorialPanel::IsStepInteractive() const
{
    if (!IsActive())
        return false;

    const TutorialStep& l_step = m_steps[m_currentStep];
    if (!l_step.checklist.empty())
        return true;
    const std::string& l_event = l_step.event;
    return l_event == "click_region" || l_event == "click_panel" || l_event == "wait_state";
}

void TutorialPanel::NotifyEvent(const std::string& _event)
{
    if (!IsActive()) return;
    const auto& l_checklist = m_steps[m_currentStep].checklist;
    for (size_t i = 0; i < l_checklist.size(); ++i)
    {
        if (i < m_checklistDone.size() && !m_checklistDone[i] && l_checklist[i].event == _event)
        {
            m_checklistDone[i] = true;
            return;
        }
    }
}

std::string TutorialPanel::GetCurrentHighlightKey() const
{
    if (!IsActive() || m_currentStep >= (int)m_steps.size())
        return "";
    
    return m_steps[m_currentStep].highlightKey;
}

void TutorialPanel::ClearRegions()
{
    m_regions.clear();
}

void TutorialPanel::ApplyForceState(const std::string& _state)
{
    if (_state.empty())
    {
        return;
    }
    // Support semicolon-delimited list of actions, e.g. "panel_close:X;panel_open:Y"
    std::string l_token;
    std::istringstream l_stream(_state);
    while (std::getline(l_stream, l_token, ';'))
    {
        // Remove leading/trailing whitespace
        auto l_start = l_token.find_first_not_of(" \t");
        auto l_end   = l_token.find_last_not_of(" \t");
        if (l_start != std::string::npos)
        {
            m_editor.GetStateRegistry()
                    .Apply(l_token.substr(l_start, l_end - l_start + 1));
        }
    }
}

bool TutorialPanel::IsStateAchieved(const std::string& _state) const
{
    if (_state.empty())
    {
        return true;
    }
    
    return m_editor.GetStateRegistry().Evaluate(_state);
}

// Resolves the ImRect for _key: checks manual registrations first,
// then falls back to the ImGui window with that title.
static bool ResolveHighlightRect(const std::string& _key,
                                  const std::map<std::string, ImRect>& _regions,
                                  ImRect& _outRect)
{
    auto l_it = _regions.find(_key);
    if (l_it != _regions.end())
    {
        _outRect = l_it->second;
        _outRect.Expand(4.0f);
        return true;
    }

    ImGuiWindow* l_win = ImGui::FindWindowByName(_key.c_str());
    if (l_win && !l_win->Hidden)
    {
        _outRect = ImRect(l_win->Pos,
                          ImVec2(l_win->Pos.x + l_win->Size.x,
                                 l_win->Pos.y + l_win->Size.y));
        _outRect.Expand(4.0f);
        return true;
    }

    return false;
}

void TutorialPanel::DrawHighlightOverlay()
{
    if (m_steps.empty() || m_currentStep >= (int)m_steps.size())
    {
        return;
    }

    const std::string& l_key = m_steps[m_currentStep].highlightKey;
    if (l_key.empty())
    {
        return;
    }

    ImRect l_rect;
    if (!ResolveHighlightRect(l_key, m_regions, l_rect))
    {
        return;
    }

    ImDrawList* l_dl = ImGui::GetForegroundDrawList();
    l_dl->AddRectFilled(l_rect.Min, l_rect.Max, IM_COL32(255, 200, 50, 40));
    float l_pulse = 0.65f + 0.35f * sinf((float)ImGui::GetTime() * 4.0f);
    ImU32 l_borderCol = IM_COL32(255, 200, 50, (int)(220 * l_pulse));
    l_dl->AddRect(l_rect.Min, l_rect.Max, l_borderCol, 4.0f, 0, 3.0f);
}

void TutorialPanel::Draw()
{
    const TutorialStep& l_step = m_steps[m_currentStep];
    const bool l_isClickRegion = (l_step.event == "click_region" && !l_step.highlightKey.empty());
    const bool l_isClickPanel  = (l_step.event == "click_panel"  && !l_step.highlightKey.empty());
    const bool l_isWaitState   = (l_step.event == "wait_state"   && !l_step.waitState.empty());
    const bool l_isChecklist   = !l_step.checklist.empty();
    const bool l_isInteractive = l_isClickRegion || l_isClickPanel || l_isWaitState || l_isChecklist;
    const bool l_isLast = (m_currentStep == (int)m_steps.size() - 1);

    // Apply force state on the first frame of a new step
    const bool l_justEntered = (m_currentStep != m_lastAppliedForceStep);
    if (l_justEntered)
    {
        ApplyForceState(l_step.forceState);
        m_lastAppliedForceStep = m_currentStep;
        // Snapshot whether the wait_state condition is already satisfied right now
        // (possibly because force_state just switched us into it). We suppress
        // auto-advance until the condition first drops false, ensuring the user
        // has to naturally achieve the state rather than having it gifted by force.
        m_waitSatisfiedAtEntry = l_isWaitState && IsStateAchieved(l_step.waitState);
        m_checklistDone.assign(l_step.checklist.size(), false);
    }

    // Auto-advance: detect a click inside the highlighted region (not on entry frame)
    if (l_isClickRegion && !l_justEntered)
    {
        ImRect l_rect;
        if (ResolveHighlightRect(l_step.highlightKey, m_regions, l_rect) && ImGui::IsMouseClicked(0))
        {
            if (l_rect.Contains(ImGui::GetMousePos()))
            {
                if (l_isLast)
                {
                    m_showPanel = false;
                }
                else
                {
                    m_currentStep++;
                }
            }
        }
    }

    // Auto-advance: detect a click anywhere inside the named ImGui panel/window
    if (l_isClickPanel && !l_justEntered)
    {
        ImGuiWindow* l_win = ImGui::FindWindowByName(l_step.highlightKey.c_str());
        if (l_win && !l_win->Hidden && ImGui::IsMouseClicked(0))
        {
            ImRect l_winRect(l_win->Pos,
                             ImVec2(l_win->Pos.x + l_win->Size.x,
                                    l_win->Pos.y + l_win->Size.y));
            if (l_winRect.Contains(ImGui::GetMousePos()))
            {
                if (l_isLast)
                {
                    m_showPanel = false;
                }
                else
                {
                    m_currentStep++;
                }
            }
        }
    }

    // Auto-advance: check required editor state.
    // Skipped on the entry frame (l_justEntered) and also suppressed while
    // m_waitSatisfiedAtEntry is true — i.e. when the condition was already met
    // on entry (e.g. because force_state put us there). The flag is cleared once
    // the condition drops to false, after which a genuine user action can satisfy it.
    if (l_isWaitState && !l_justEntered)
    {
        bool l_conditionMet = IsStateAchieved(l_step.waitState);
        if (!l_conditionMet)
            m_waitSatisfiedAtEntry = false;  // condition dropped — next true will be genuine
        if (l_conditionMet && !m_waitSatisfiedAtEntry)
        {
            if (l_isLast)
            {
                m_showPanel = false;
            }
            else
            {
                m_currentStep++;
            }
        }
    }

    // Auto-advance: all checklist items fulfilled
    if (l_isChecklist && !l_justEntered)
    {
        bool l_allDone = !m_checklistDone.empty() &&
                         std::all_of(m_checklistDone.begin(), m_checklistDone.end(), [](bool b){ return b; });
        if (l_allDone)
        {
            if (l_isLast) m_showPanel = false;
            else          m_currentStep++;
        }
    }

    // Full-screen dim window for non-interactive steps ("next" button steps)
    // Rendered before the tutorial window so it sits below it
    // but above the editor panels (which were drawn in DrawEditorUI before this call).
    // Because it captures mouse input, editor panels behind it cannot be clicked.
    // Skip the dim when the text editor is active so the user can freely interact with it.
    bool l_isTextEdit = m_editor.GetState() == Editor::State::TextEdit;
    if (!l_isInteractive && !l_isTextEdit)
    {
        ImGuiIO& l_io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(l_io.DisplaySize);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.55f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("##TutorialDim", nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing);
        ImGui::End();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    }

    DrawHighlightOverlay();

    // Calculate dynamic window size based on content
    ImVec2 l_windowSize = ImGui::GetIO().DisplaySize;
    float l_maxWidth = l_windowSize.x * 0.35f;  // Use up to 35% of screen width
    float l_maxHeight = l_windowSize.y * 0.6f;  // Use up to 60% of screen height
    ImVec2 l_contentSize = ImGui::CalcTextSize(l_step.title.c_str(), nullptr, false, l_maxWidth);
    l_contentSize.y += 60.0f;  // Add space for title styling, separator, and buttons

    // Add space for image if present
    if (!l_step.imagePath.empty())
    {
        auto l_imgIt = m_imageCache.find(l_step.imagePath);
        if (l_imgIt != m_imageCache.end() && l_imgIt->second)
        {
            auto& l_tex = l_imgIt->second;
            glm::ivec2 l_texSize = l_tex->Size();
            float l_scale = std::min(l_maxWidth / (float)l_texSize.x,
                                     (l_maxHeight * 0.4f) / (float)l_texSize.y);
            l_contentSize.y += (l_texSize.y * l_scale) + 20.0f;
        }
    }

    // Add space for body text (estimate based on text length and width)
    size_t l_bodyLines = std::max(3, (int)(l_step.body.length() / 40));
    l_contentSize.y += (l_bodyLines * 20.0f) + 20.0f;

    // Clamp to max size
    l_contentSize.x = std::min(l_maxWidth, std::max(460.0f, l_contentSize.x + 40.0f));
    l_contentSize.y = std::min(l_maxHeight, l_contentSize.y + 40.0f);

    ImGui::SetWindowSize(l_contentSize, ImGuiCond_FirstUseEver);
    ImGui::SetWindowPos(ImVec2(30, 80), ImGuiCond_FirstUseEver);

    if (!ImGui::BeginChild(m_tutorialTitle.empty() ? "Tutorial" : m_tutorialTitle.c_str(), ImVec2(0, 0), true))
    {
        ImGui::EndChild();
        return;
    }

    // Step title
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.35f, 1.0f));
    ImGui::TextWrapped("%s", l_step.title.c_str());
    ImGui::PopStyleColor();

    // Step counter
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 70);
    ImGui::TextDisabled("Step %d / %d", m_currentStep + 1, (int)m_steps.size());

    ImGui::Separator();

    // Optional image
    float l_bodyHeight = ImGui::GetContentRegionAvail().y - 40.0f; // reserve room for nav buttons

    if (!l_step.imagePath.empty())
    {
        auto l_imgIt = m_imageCache.find(l_step.imagePath);
        if (l_imgIt != m_imageCache.end() && l_imgIt->second)
        {
            auto& l_tex = l_imgIt->second;
            glm::ivec2 l_texSize = l_tex->Size();

            float l_maxWidth  = ImGui::GetContentRegionAvail().x;
            float l_maxHeight = l_bodyHeight * 0.45f;
            float l_scale     = std::min(l_maxWidth  / (float)l_texSize.x,
                                         l_maxHeight / (float)l_texSize.y);
            ImVec2 l_drawSize(l_texSize.x * l_scale, l_texSize.y * l_scale);

            // Centre the image
            float l_pad = (l_maxWidth - l_drawSize.x) * 0.5f;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + l_pad);

            ImTextureID l_texId = (ImTextureID)(uintptr_t)l_tex->GetID();
            ImGui::Image(l_texId, l_drawSize);
            ImGui::Spacing();

            l_bodyHeight -= (l_drawSize.y + ImGui::GetStyle().ItemSpacing.y * 2.0f);
        }
    }

    // Scrollable body text
    ImGui::InputTextMultiline("##body",
                              &m_steps[m_currentStep].body,
                              ImVec2(-1.0f, l_bodyHeight - 40.0f),
                              ImGuiInputTextFlags_ReadOnly);

    ImGui::Separator();

    // Navigation buttons
    bool l_isFirst = (m_currentStep == 0);

    if (l_isClickRegion)
    {
        // User advances by clicking the highlighted area – show progress bar and hint.
        float l_progress = m_steps.size() > 1
            ? (float)m_currentStep / (float)(m_steps.size() - 1) : 1.0f;
        ImGui::ProgressBar(l_progress, ImVec2(-1.0f, 0.0f), "");
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        ImGui::TextWrapped("Click the highlighted area to continue...");
        ImGui::PopStyleColor();
    }
    else if (l_isClickPanel)
    {
        // User advances by clicking anywhere inside the named panel/window.
        float l_progress = m_steps.size() > 1
            ? (float)m_currentStep / (float)(m_steps.size() - 1) : 1.0f;
        ImGui::ProgressBar(l_progress, ImVec2(-1.0f, 0.0f), "");
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        ImGui::TextWrapped("Click the \"%s\" panel to continue...", l_step.highlightKey.c_str());
        ImGui::PopStyleColor();
    }
    else if (l_isWaitState)
    {
        // Tutorial is waiting for a specific editor state – show progress bar and hint.
        float l_progress = m_steps.size() > 1
            ? (float)m_currentStep / (float)(m_steps.size() - 1) : 1.0f;
        ImGui::ProgressBar(l_progress, ImVec2(-1.0f, 0.0f), "");
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        ImGui::TextWrapped("Waiting for: %s", l_step.waitState.c_str());
        ImGui::PopStyleColor();
    }
    else if (l_isChecklist)
    {
        // Show each checklist item with its completion state.
        for (size_t i = 0; i < l_step.checklist.size(); ++i)
        {
            bool l_done = (i < m_checklistDone.size()) && m_checklistDone[i];
            ImGui::PushStyleColor(ImGuiCol_Text, l_done
                ? ImVec4(0.4f, 0.9f, 0.4f, 1.0f)
                : ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
            ImGui::Text("%s %s", l_done ? "[x]" : "[ ]", l_step.checklist[i].label.c_str());
            ImGui::PopStyleColor();
        }
        int   l_doneCount = (int)std::count(m_checklistDone.begin(), m_checklistDone.end(), true);
        float l_progress  = l_step.checklist.empty() ? 1.0f
            : (float)l_doneCount / (float)l_step.checklist.size();
        ImGui::ProgressBar(l_progress, ImVec2(-1.0f, 0.0f), "");
    }
    else
    {
        if (l_isFirst)
            ImGui::BeginDisabled();
        if (ImGui::Button("< Prev", ImVec2(80, 0)))
            m_currentStep--;
        if (l_isFirst)
            ImGui::EndDisabled();

        ImGui::SameLine();

        // Progress bar
        float l_progress = m_steps.size() > 1
            ? (float)m_currentStep / (float)(m_steps.size() - 1)
            : 1.0f;
        ImGui::ProgressBar(l_progress, ImVec2(-85.0f, 0.0f), "");

        ImGui::SameLine();

        if (l_isLast)
        {
            if (ImGui::Button("Finish", ImVec2(80, 0)))
                m_showPanel = false;
        }
        else
        {
            if (ImGui::Button("Next >", ImVec2(80, 0)))
                m_currentStep++;
        }
    }

    ImGui::EndChild();
}
