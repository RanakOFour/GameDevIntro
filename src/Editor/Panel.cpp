#include "Editor/Panel.h"
#include "Editor/Editor.h"
#include "Editor/StateRegistry.h"

Panel::Panel(std::string _name, std::weak_ptr<Editor> _editor)
: m_panelTitle(_name)
, m_editor(_editor)
, m_showPanel(false)
, m_filterString("")
{
    // Self-register conditions and actions so the StateRegistry can observe
    // and control this panel without knowing its concrete type.
    // Raw pointer capture is safe: this panel's lifetime is nested inside the
    // Editor that owns the registry (SceneEditTab owns panels by value, and
    // Editor owns SceneEditTab; Draw() is never called after destruction).
    if (auto l_editor = m_editor.lock())
    {
        Panel* l_self = this;
        StateRegistry& l_reg = l_editor->GetStateRegistry();
        l_reg.RegisterCondition("panel_open:" + m_panelTitle, [l_self]{ return l_self->m_showPanel; });
        l_reg.RegisterAction("panel:" + m_panelTitle,         [l_self]{ l_self->m_showPanel = true; });
    }
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