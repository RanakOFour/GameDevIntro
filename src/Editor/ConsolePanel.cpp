#include "Editor/ConsolePanel.h"
#include "Editor/Editor.h"

#include "imgui/imgui.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

ConsolePanel::ConsolePanel(Editor& _editor)
    : Panel("Console", _editor)
{
    // Register listener with the engine log system.
    RanakEngine::Log::Manager::AddListener(
        [this](RanakEngine::Log::MessageContent::Severity _sev, const std::string& _msg) {
            std::lock_guard<std::mutex> l_lk(m_entriesMutex);
            m_entries.push_back({_sev, _msg});
        }
    );
}

ConsolePanel::~ConsolePanel()
{
}

void ConsolePanel::ClearLog()
{
    std::lock_guard<std::mutex> l_lk(m_entriesMutex);
    m_entries.clear();
}

void ConsolePanel::Draw()
{
    // Toolbar row
    if (ImGui::Button("Clear"))
        ClearLog();

    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_autoScroll);

    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();

    // Severity filter toggles
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    ImGui::Checkbox("DBG", &m_showDebug);
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::Checkbox("LOG", &m_showNormal);
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.9f, 0.3f, 1.0f));
    ImGui::Checkbox("WRN", &m_showWarning);
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
    ImGui::Checkbox("ERR", &m_showError);
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::SetNextItemWidth(200.0f);
    ImGui::InputText("##Filter", &m_filterString);

    ImGui::Separator();

    // Scrollable log area
    if (ImGui::BeginChild("ConsoleLogRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        std::lock_guard<std::mutex> l_lk(m_entriesMutex);

        for (const auto& l_entry : m_entries)
        {
            // Severity filter
            using Sev = RanakEngine::Log::MessageContent::Severity;
            if (l_entry.severity == Sev::DEBUG   && !m_showDebug)   continue;
            if (l_entry.severity == Sev::NORMAL  && !m_showNormal)  continue;
            if (l_entry.severity == Sev::WARNING && !m_showWarning) continue;
            if (l_entry.severity == Sev::ERRORLOG && !m_showError)  continue;

            // Text filter
            if (!m_filterString.empty())
            {
                if (l_entry.message.find(m_filterString) == std::string::npos)
                    continue;
            }

            ImVec4 l_color;
            const char* l_prefix;
            switch (l_entry.severity)
            {
                case Sev::DEBUG:    l_color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f); l_prefix = "[DBG] "; break;
                case Sev::NORMAL:   l_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); l_prefix = "[LOG] "; break;
                case Sev::WARNING:  l_color = ImVec4(1.0f, 0.9f, 0.3f, 1.0f); l_prefix = "[WRN] "; break;
                case Sev::ERRORLOG: l_color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f); l_prefix = "[ERR] "; break;
                default:            l_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); l_prefix = ""; break;
            }

            ImGui::PushStyleColor(ImGuiCol_Text, l_color);
            ImGui::TextUnformatted((std::string(l_prefix) + l_entry.message).c_str());
            ImGui::PopStyleColor();
        }

        if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
}
