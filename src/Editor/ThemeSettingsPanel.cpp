#include "Editor/ThemeSettingsPanel.h"
#include "Editor/ThemeManager.h"

#include "imgui/imgui.h"

#include <cstring>

ThemeSettingsPanel::ThemeSettingsPanel(Editor& _editor, ThemeManager& _themeManager)
    : Panel("Theme Settings", _editor)
    , m_themeManager(_themeManager)
{
    m_showPanel = false;
}

void ThemeSettingsPanel::Draw()
{
    auto& l_presets = m_themeManager.GetPresets();
    int l_active = m_themeManager.GetActiveIndex();

    // Preset selector
    ImGui::Text("Preset:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##PresetCombo", l_presets[l_active].name.c_str()))
    {
        for (int i = 0; i < (int)l_presets.size(); i++)
        {
            bool l_selected = (i == l_active);
            if (ImGui::Selectable(l_presets[i].name.c_str(), l_selected))
            {
                m_themeManager.ApplyPreset(i);
                m_themeManager.SavePresets();
            }
            if (l_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Colour editors for the active preset
    ThemePreset& p = l_presets[l_active];
    bool l_changed = false;

    ImGui::Text("Colours");
    ImGui::Spacing();

    l_changed |= ImGui::ColorEdit4("Window Background",  &p.windowBg.x,       ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Child Background",   &p.childBg.x,        ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Title Bar",          &p.titleBg.x,        ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Title Bar Active",   &p.titleBgActive.x,  ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Menu Bar",           &p.menuBarBg.x,      ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Tab",                &p.tab.x,            ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Tab Selected",       &p.tabSelected.x,    ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Tab Hovered",        &p.tabHovered.x,     ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Header",             &p.header.x,         ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Header Hovered",     &p.headerHovered.x,  ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Header Active",      &p.headerActive.x,   ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Frame Background",   &p.frameBg.x,        ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Frame Bg Hovered",   &p.frameBgHovered.x, ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Frame Bg Active",    &p.frameBgActive.x,  ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Button",             &p.button.x,         ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Button Hovered",     &p.buttonHovered.x,  ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Button Active",      &p.buttonActive.x,   ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Text",               &p.text.x,           ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Separator",          &p.separator.x,      ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Border",             &p.border.x,         ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Popup Background",   &p.popupBg.x,       ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Scrollbar Bg",       &p.scrollbarBg.x,    ImGuiColorEditFlags_NoInputs);
    l_changed |= ImGui::ColorEdit4("Scrollbar Grab",     &p.scrollbarGrab.x,  ImGuiColorEditFlags_NoInputs);

    if (l_changed)
    {
        m_themeManager.ApplyPreset(p);
        m_themeManager.SavePresets();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Reset to defaults
    if (ImGui::Button("Reset to Default"))
    {
        p = ThemeManager::DefaultPreset();
        p.name = l_presets[l_active].name; // keep user's name
        m_themeManager.ApplyPreset(p);
        m_themeManager.SavePresets();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Create new preset
    ImGui::Text("New Preset:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(180.0f);
    ImGui::InputText("##NewPresetName", m_newPresetName, sizeof(m_newPresetName));
    ImGui::SameLine();
    if (ImGui::Button("Create") && m_newPresetName[0] != '\0')
    {
        ThemePreset l_newPreset = l_presets[l_active]; // copy current
        l_newPreset.name = m_newPresetName;
        l_presets.push_back(l_newPreset);
        m_themeManager.ApplyPreset((int)l_presets.size() - 1);
        m_themeManager.SavePresets();
        m_newPresetName[0] = '\0';
    }

    // Delete current preset (only if more than one remains)
    if ((int)l_presets.size() > 1)
    {
        ImGui::SameLine();
        if (ImGui::Button("Delete Current"))
        {
            l_presets.erase(l_presets.begin() + l_active);
            int l_newActive = (l_active >= (int)l_presets.size()) ? (int)l_presets.size() - 1 : l_active;
            m_themeManager.ApplyPreset(l_newActive);
            m_themeManager.SavePresets();
        }
    }
}
