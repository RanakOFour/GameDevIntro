#include "Editor/Panel.h"
#include "Editor/Editor.h"

Panel::Panel(std::string _name, std::weak_ptr<Editor> _editor)
: m_panelTitle(_name)
, m_editor(_editor)
, m_showPanel(false)
, m_filterString("")
{
}

void Panel::DrawAsWindow(ImGuiWindowFlags _flags)
{
    if (!m_showPanel) return;

    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(m_panelTitle.c_str(), &m_showPanel, _flags))
    {
        Draw();
    }
    ImGui::End();
}

void Panel::DrawAsChild(ImGuiChildFlags _flags)
{
    if (!m_showPanel) return;

    if (ImGui::BeginChild(m_panelTitle.c_str(), ImVec2(0, 0), true, _flags))
    {
        Draw();
    }
    ImGui::EndChild();
}