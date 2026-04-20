#include "Editor/BuiltinCategories.h"

#include "RanakEngine/Asset/LuaFile.h"

#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <cstdio>

const std::vector<BuiltinCategories::Entry>& BuiltinCategories::GetEntries()
{
    static const std::vector<Entry> l_entries = {
        {
            "Transform",
            "return Category {\n"
            "    Position = Vector2(0.0),\n"
            "    Layer    = 0,\n"
            "    Rotation = 0.0,\n"
            "    Scale    = Vector2(1.0)\n"
            "}\n"
        },
        {
            "Model",
            "return Category {\n"
            "    modelPath = \"\",\n"
            "    asset     = Field(nil, { hidden = true })\n"
            "}\n"
        },
        {
            "Texture",
            "return Category {\n"
            "    texturePath = \"\",\n"
            "    asset       = Field(nil, { hidden = true })\n"
            "}\n"
        },
        {
            "Shader",
            "return Category {\n"
            "    vertexshaderPath   = \"\",\n"
            "    fragmentshaderPath = \"\",\n"
            "    asset              = Field(nil, { hidden = true })\n"
            "}\n"
        },
        {
            "PhysicsBody",
            "return Category {\n"
            "    bodyType    = \"dynamic\",\n"
            "    density     = 1.0,\n"
            "    friction    = 0.3,\n"
            "    restitution = 0.1,\n"
            "    _body       = Field(nil, { hidden = true })\n"
            "}\n"
        },
        // ── UI categories ────────────────────────────────────────────────
        {
            "UIText",
            "return Category {\n"
            "    text      = \"\",\n"
            "    fontSize  = 16.0,\n"
            "    colorR    = 1.0,\n"
            "    colorG    = 1.0,\n"
            "    colorB    = 1.0,\n"
            "    colorA    = 1.0,\n"
            "    anchorX   = 0.0,\n"
            "    anchorY   = 0.0,\n"
            "    visible   = true\n"
            "}\n"
        },
        {
            "UIButton",
            "return Category {\n"
            "    label     = \"Button\",\n"
            "    width     = 120.0,\n"
            "    height    = 40.0,\n"
            "    colorR    = 0.2,\n"
            "    colorG    = 0.4,\n"
            "    colorB    = 0.7,\n"
            "    colorA    = 1.0,\n"
            "    hoverR    = 0.3,\n"
            "    hoverG    = 0.5,\n"
            "    hoverB    = 0.8,\n"
            "    hoverA    = 1.0,\n"
            "    anchorX   = 0.0,\n"
            "    anchorY   = 0.0,\n"
            "    pressed   = Field(false, { hidden = true }),\n"
            "    hovered   = Field(false, { hidden = true }),\n"
            "    visible   = true\n"
            "}\n"
        },
        {
            "UIPanel",
            "return Category {\n"
            "    width     = 200.0,\n"
            "    height    = 150.0,\n"
            "    colorR    = 0.12,\n"
            "    colorG    = 0.12,\n"
            "    colorB    = 0.16,\n"
            "    colorA    = 0.9,\n"
            "    anchorX   = 0.0,\n"
            "    anchorY   = 0.0,\n"
            "    visible   = true\n"
            "}\n"
        },
        {
            "UIImage",
            "return Category {\n"
            "    texturePath = \"\",\n"
            "    width       = 64.0,\n"
            "    height      = 64.0,\n"
            "    tintR       = 1.0,\n"
            "    tintG       = 1.0,\n"
            "    tintB       = 1.0,\n"
            "    tintA       = 1.0,\n"
            "    anchorX     = 0.0,\n"
            "    anchorY     = 0.0,\n"
            "    visible     = true,\n"
            "    asset       = Field(nil, { hidden = true })\n"
            "}\n"
        },
    };
    return l_entries;
}

std::string BuiltinCategories::GetDataDir()
{
#if defined(_WIN32)
    const char* l_appDataRaw = std::getenv("APPDATA");
    const std::string l_appData = l_appDataRaw ? l_appDataRaw : "";
    std::filesystem::path l_base = l_appData.empty() ? std::filesystem::path(".") : std::filesystem::path(l_appData);
#else
    // Prefer XDG_DATA_HOME, fall back to ~/.local/share
    const char* l_xdgRaw  = std::getenv("XDG_DATA_HOME");
    const char* l_homeRaw = std::getenv("HOME");
    const std::string l_xdg  = l_xdgRaw  ? l_xdgRaw  : "";
    const std::string l_home = l_homeRaw ? l_homeRaw : "";
    std::filesystem::path l_base;

    if (!l_xdg.empty())
    {
        l_base = std::filesystem::path(l_xdg);
    }
    else if (!l_home.empty())
    {
        l_base = std::filesystem::path(l_home) / ".local" / "share";
    }
    else
    {
        l_base = std::filesystem::path(".");
    }
#endif

    return (l_base / "GameDevIntro" / "Categories").string();
}

bool BuiltinCategories::IsBuiltin(const std::string& _name)
{
    for (const auto& entry : GetEntries())
    {
        if (_name == entry.name)
        {
            return true;
        }
    }

    return false;
}

void BuiltinCategories::Load(RE::EngineContents& _contents)
{
    std::filesystem::path l_dir(GetDataDir());

    if (!std::filesystem::exists(l_dir))
    {
        std::filesystem::create_directories(l_dir);
    }

    for (const auto& entry : GetEntries())
    {
        std::filesystem::path l_path = l_dir / (entry.name + ".lua");

        // Write the file only when it is missing
        if (!std::filesystem::exists(l_path))
        {
            std::ofstream l_out(l_path);
            if (!l_out)
            {
                printf("[BuiltinCategories] WARNING: could not write %s\n",
                       l_path.string().c_str());
                continue;
            }
            l_out << entry.source;
            printf("BuiltinCategories: Created %s\n", l_path.string().c_str());
        }

        auto l_file = _contents.resources->Load<RE::Asset::LuaFile>(l_path.string());
        _contents.core->GetLuaContext()->CreateCategory(l_file);
    }
}
