#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "imgui/imgui.h"

#include <string>
#include <vector>

/**
 * @struct ThemePreset
 * @brief A named collection of ImGui style colours that can be applied at runtime.
 */
struct ThemePreset
{
    std::string name; ///< Display name of the preset (e.g. "Dark Blue").

    ImVec4 windowBg;       ///< ImGuiCol_WindowBg
    ImVec4 childBg;        ///< ImGuiCol_ChildBg
    ImVec4 titleBg;        ///< ImGuiCol_TitleBg
    ImVec4 titleBgActive;  ///< ImGuiCol_TitleBgActive
    ImVec4 menuBarBg;      ///< ImGuiCol_MenuBarBg
    ImVec4 tab;            ///< ImGuiCol_Tab
    ImVec4 tabSelected;    ///< ImGuiCol_TabSelected
    ImVec4 tabHovered;     ///< ImGuiCol_TabHovered
    ImVec4 header;         ///< ImGuiCol_Header
    ImVec4 headerHovered;  ///< ImGuiCol_HeaderHovered
    ImVec4 headerActive;   ///< ImGuiCol_HeaderActive
    ImVec4 frameBg;        ///< ImGuiCol_FrameBg
    ImVec4 frameBgHovered; ///< ImGuiCol_FrameBgHovered
    ImVec4 frameBgActive;  ///< ImGuiCol_FrameBgActive
    ImVec4 button;         ///< ImGuiCol_Button
    ImVec4 buttonHovered;  ///< ImGuiCol_ButtonHovered
    ImVec4 buttonActive;   ///< ImGuiCol_ButtonActive
    ImVec4 text;           ///< ImGuiCol_Text
    ImVec4 separator;      ///< ImGuiCol_Separator
    ImVec4 border;         ///< ImGuiCol_Border
    ImVec4 popupBg;        ///< ImGuiCol_PopupBg
    ImVec4 scrollbarBg;    ///< ImGuiCol_ScrollbarBg
    ImVec4 scrollbarGrab;  ///< ImGuiCol_ScrollbarGrab
};

/**
 * @class ThemeManager
 * @brief Manages colour theme presets, persisted as JSON in shared storage.
 *
 * Presets are stored in `$XDG_DATA_HOME/GameDevIntro/Themes.json` (Linux)
 * or `%APPDATA%\GameDevIntro\Themes.json` (Windows).
 * A default "Dark Blue" preset matching the project selection screen palette
 * is always available.
 */
class ThemeManager
{
public:
    ThemeManager();

    /** @brief Loads presets from the JSON file on disk. */
    void LoadPresets();
    /** @brief Saves all presets (including user presets) to the JSON file. */
    void SavePresets() const;

    /** @brief Applies the given preset to the current ImGui style. */
    void ApplyPreset(const ThemePreset& _preset);
    /** @brief Applies the preset at the given index. */
    void ApplyPreset(int _index);

    /** @brief Returns the list of all loaded presets. */
    const std::vector<ThemePreset>& GetPresets() const { return m_presets; }
    /** @brief Returns a mutable reference to the presets list. */
    std::vector<ThemePreset>& GetPresets() { return m_presets; }

    /** @brief Returns the index of the currently active preset, or -1. */
    int GetActiveIndex() const { return m_activeIndex; }
    /** @brief Sets the active preset index (does not apply; call ApplyPreset). */
    void SetActiveIndex(int _index) { m_activeIndex = _index; }

    /** @brief Returns the built-in default "Dark Blue" preset. */
    static ThemePreset DefaultPreset();

private:
    /** @brief Returns the path to the shared Themes.json file. */
    static std::string GetThemesPath();

    std::vector<ThemePreset> m_presets;
    int m_activeIndex = 0;
};

#endif
