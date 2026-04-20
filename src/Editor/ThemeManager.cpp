#include "Editor/ThemeManager.h"

#include "json/json.hpp"
using json = nlohmann::json;

#include <cstdlib>
#include <filesystem>
#include <fstream>

static json Vec4ToJson(const ImVec4& v)
{
    return json::array({v.x, v.y, v.z, v.w});
}

static ImVec4 JsonToVec4(const json& j, const ImVec4& fallback = ImVec4(0,0,0,1))
{
    if (!j.is_array() || j.size() < 4)
        return fallback;
    return ImVec4(j[0].get<float>(), j[1].get<float>(), j[2].get<float>(), j[3].get<float>());
}

static json PresetToJson(const ThemePreset& p)
{
    json j;
    j["name"]           = p.name;
    j["windowBg"]       = Vec4ToJson(p.windowBg);
    j["childBg"]        = Vec4ToJson(p.childBg);
    j["titleBg"]        = Vec4ToJson(p.titleBg);
    j["titleBgActive"]  = Vec4ToJson(p.titleBgActive);
    j["menuBarBg"]      = Vec4ToJson(p.menuBarBg);
    j["tab"]            = Vec4ToJson(p.tab);
    j["tabSelected"]    = Vec4ToJson(p.tabSelected);
    j["tabHovered"]     = Vec4ToJson(p.tabHovered);
    j["header"]         = Vec4ToJson(p.header);
    j["headerHovered"]  = Vec4ToJson(p.headerHovered);
    j["headerActive"]   = Vec4ToJson(p.headerActive);
    j["frameBg"]        = Vec4ToJson(p.frameBg);
    j["frameBgHovered"] = Vec4ToJson(p.frameBgHovered);
    j["frameBgActive"]  = Vec4ToJson(p.frameBgActive);
    j["button"]         = Vec4ToJson(p.button);
    j["buttonHovered"]  = Vec4ToJson(p.buttonHovered);
    j["buttonActive"]   = Vec4ToJson(p.buttonActive);
    j["text"]           = Vec4ToJson(p.text);
    j["separator"]      = Vec4ToJson(p.separator);
    j["border"]         = Vec4ToJson(p.border);
    j["popupBg"]        = Vec4ToJson(p.popupBg);
    j["scrollbarBg"]    = Vec4ToJson(p.scrollbarBg);
    j["scrollbarGrab"]  = Vec4ToJson(p.scrollbarGrab);
    return j;
}

static ThemePreset JsonToPreset(const json& j)
{
    ThemePreset p = ThemeManager::DefaultPreset(); // fallback values
    if (j.contains("name"))           p.name           = j["name"].get<std::string>();
    if (j.contains("windowBg"))       p.windowBg       = JsonToVec4(j["windowBg"]);
    if (j.contains("childBg"))        p.childBg        = JsonToVec4(j["childBg"]);
    if (j.contains("titleBg"))        p.titleBg        = JsonToVec4(j["titleBg"]);
    if (j.contains("titleBgActive"))  p.titleBgActive  = JsonToVec4(j["titleBgActive"]);
    if (j.contains("menuBarBg"))      p.menuBarBg      = JsonToVec4(j["menuBarBg"]);
    if (j.contains("tab"))            p.tab            = JsonToVec4(j["tab"]);
    if (j.contains("tabSelected"))    p.tabSelected    = JsonToVec4(j["tabSelected"]);
    if (j.contains("tabHovered"))     p.tabHovered     = JsonToVec4(j["tabHovered"]);
    if (j.contains("header"))         p.header         = JsonToVec4(j["header"]);
    if (j.contains("headerHovered"))  p.headerHovered  = JsonToVec4(j["headerHovered"]);
    if (j.contains("headerActive"))   p.headerActive   = JsonToVec4(j["headerActive"]);
    if (j.contains("frameBg"))        p.frameBg        = JsonToVec4(j["frameBg"]);
    if (j.contains("frameBgHovered")) p.frameBgHovered = JsonToVec4(j["frameBgHovered"]);
    if (j.contains("frameBgActive"))  p.frameBgActive  = JsonToVec4(j["frameBgActive"]);
    if (j.contains("button"))         p.button         = JsonToVec4(j["button"]);
    if (j.contains("buttonHovered"))  p.buttonHovered  = JsonToVec4(j["buttonHovered"]);
    if (j.contains("buttonActive"))   p.buttonActive   = JsonToVec4(j["buttonActive"]);
    if (j.contains("text"))           p.text           = JsonToVec4(j["text"]);
    if (j.contains("separator"))      p.separator      = JsonToVec4(j["separator"]);
    if (j.contains("border"))         p.border         = JsonToVec4(j["border"]);
    if (j.contains("popupBg"))        p.popupBg        = JsonToVec4(j["popupBg"]);
    if (j.contains("scrollbarBg"))    p.scrollbarBg    = JsonToVec4(j["scrollbarBg"]);
    if (j.contains("scrollbarGrab"))  p.scrollbarGrab  = JsonToVec4(j["scrollbarGrab"]);
    return p;
}

std::string ThemeManager::GetThemesPath()
{
#if defined(_WIN32)
    const char* l_appDataRaw = std::getenv("APPDATA");
    std::string l_appData = l_appDataRaw ? l_appDataRaw : "";
    std::filesystem::path l_base = l_appData.empty()
        ? std::filesystem::path(".")
        : std::filesystem::path(l_appData);
#else
    const char* l_xdgRaw  = std::getenv("XDG_DATA_HOME");
    const char* l_homeRaw = std::getenv("HOME");
    std::string l_xdg  = l_xdgRaw  ? l_xdgRaw  : "";
    std::string l_home = l_homeRaw ? l_homeRaw : "";
    std::filesystem::path l_base;
    if (!l_xdg.empty())
        l_base = std::filesystem::path(l_xdg);
    else if (!l_home.empty())
        l_base = std::filesystem::path(l_home) / ".local" / "share";
    else
        l_base = std::filesystem::path(".");
#endif
    return (l_base / "GameDevIntro" / "Themes.json").string();
}

ThemePreset ThemeManager::DefaultPreset()
{
    ThemePreset p;
    p.name           = "Dark Blue";
    p.windowBg       = ImVec4(0.09f, 0.09f, 0.12f, 1.0f);
    p.childBg        = ImVec4(0.11f, 0.11f, 0.15f, 1.0f);
    p.titleBg        = ImVec4(0.08f, 0.08f, 0.10f, 1.0f);
    p.titleBgActive  = ImVec4(0.12f, 0.14f, 0.22f, 1.0f);
    p.menuBarBg      = ImVec4(0.09f, 0.09f, 0.12f, 1.0f);
    p.tab            = ImVec4(0.12f, 0.12f, 0.17f, 1.0f);
    p.tabSelected    = ImVec4(0.20f, 0.22f, 0.34f, 1.0f);
    p.tabHovered     = ImVec4(0.22f, 0.26f, 0.40f, 1.0f);
    p.header         = ImVec4(0.17f, 0.19f, 0.28f, 1.0f);
    p.headerHovered  = ImVec4(0.21f, 0.25f, 0.40f, 1.0f);
    p.headerActive   = ImVec4(0.20f, 0.38f, 0.62f, 1.0f);
    p.frameBg        = ImVec4(0.13f, 0.13f, 0.18f, 1.0f);
    p.frameBgHovered = ImVec4(0.18f, 0.18f, 0.26f, 1.0f);
    p.frameBgActive  = ImVec4(0.20f, 0.22f, 0.34f, 1.0f);
    p.button         = ImVec4(0.17f, 0.19f, 0.26f, 1.0f);
    p.buttonHovered  = ImVec4(0.22f, 0.26f, 0.38f, 1.0f);
    p.buttonActive   = ImVec4(0.20f, 0.40f, 0.68f, 1.0f);
    p.text           = ImVec4(0.90f, 0.92f, 0.96f, 1.0f);
    p.separator      = ImVec4(0.22f, 0.26f, 0.38f, 1.0f);
    p.border         = ImVec4(0.18f, 0.20f, 0.30f, 1.0f);
    p.popupBg        = ImVec4(0.10f, 0.10f, 0.14f, 1.0f);
    p.scrollbarBg    = ImVec4(0.08f, 0.08f, 0.11f, 1.0f);
    p.scrollbarGrab  = ImVec4(0.22f, 0.24f, 0.34f, 1.0f);
    return p;
}

ThemeManager::ThemeManager()
{
    m_presets.push_back(DefaultPreset());
    LoadPresets();
}

void ThemeManager::LoadPresets()
{
    std::string l_path = GetThemesPath();
    std::ifstream l_file(l_path);
    if (!l_file.is_open())
        return;

    json l_root;
    try { l_file >> l_root; }
    catch (json::parse_error&) { return; }

    if (!l_root.contains("presets") || !l_root["presets"].is_array())
        return;

    // Replace the default list with saved presets.
    m_presets.clear();
    for (const auto& j : l_root["presets"])
        m_presets.push_back(JsonToPreset(j));

    // Ensure at least the default preset exists.
    if (m_presets.empty())
        m_presets.push_back(DefaultPreset());

    m_activeIndex = l_root.value("activeIndex", 0);
    if (m_activeIndex < 0 || m_activeIndex >= (int)m_presets.size())
        m_activeIndex = 0;
}

void ThemeManager::SavePresets() const
{
    std::string l_path = GetThemesPath();
    std::filesystem::create_directories(std::filesystem::path(l_path).parent_path());

    std::ofstream l_file(l_path, std::ios::trunc);
    if (!l_file.is_open())
        return;

    json l_root;
    l_root["activeIndex"] = m_activeIndex;
    json l_arr = json::array();
    for (const auto& p : m_presets)
        l_arr.push_back(PresetToJson(p));
    l_root["presets"] = l_arr;

    l_file << l_root.dump(4);
}

void ThemeManager::ApplyPreset(const ThemePreset& _preset)
{
    ImGuiStyle& s = ImGui::GetStyle();
    s.Colors[ImGuiCol_WindowBg]       = _preset.windowBg;
    s.Colors[ImGuiCol_ChildBg]        = _preset.childBg;
    s.Colors[ImGuiCol_TitleBg]        = _preset.titleBg;
    s.Colors[ImGuiCol_TitleBgActive]  = _preset.titleBgActive;
    s.Colors[ImGuiCol_MenuBarBg]      = _preset.menuBarBg;
    s.Colors[ImGuiCol_Tab]            = _preset.tab;
    s.Colors[ImGuiCol_TabSelected]    = _preset.tabSelected;
    s.Colors[ImGuiCol_TabHovered]     = _preset.tabHovered;
    s.Colors[ImGuiCol_Header]         = _preset.header;
    s.Colors[ImGuiCol_HeaderHovered]  = _preset.headerHovered;
    s.Colors[ImGuiCol_HeaderActive]   = _preset.headerActive;
    s.Colors[ImGuiCol_FrameBg]        = _preset.frameBg;
    s.Colors[ImGuiCol_FrameBgHovered] = _preset.frameBgHovered;
    s.Colors[ImGuiCol_FrameBgActive]  = _preset.frameBgActive;
    s.Colors[ImGuiCol_Button]         = _preset.button;
    s.Colors[ImGuiCol_ButtonHovered]  = _preset.buttonHovered;
    s.Colors[ImGuiCol_ButtonActive]   = _preset.buttonActive;
    s.Colors[ImGuiCol_Text]           = _preset.text;
    s.Colors[ImGuiCol_Separator]      = _preset.separator;
    s.Colors[ImGuiCol_Border]         = _preset.border;
    s.Colors[ImGuiCol_PopupBg]        = _preset.popupBg;
    s.Colors[ImGuiCol_ScrollbarBg]    = _preset.scrollbarBg;
    s.Colors[ImGuiCol_ScrollbarGrab]  = _preset.scrollbarGrab;
}

void ThemeManager::ApplyPreset(int _index)
{
    if (_index < 0 || _index >= (int)m_presets.size())
        return;
    m_activeIndex = _index;
    ApplyPreset(m_presets[_index]);
}
