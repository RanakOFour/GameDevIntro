#include "Editor/Scene/BuiltIn/BuiltinCategories.h"
#include "RanakEngine/Assets.h"

#include <filesystem>
#include <fstream>
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
            "    bodyType        = Field(\"dynamic\", { isEnum = true, enumOptions = {\"dynamic\", \"static\", \"kinematic\"} }),\n"
            "    shape           = Field(\"square\",  { isEnum = true, enumOptions = {\"square\", \"circle\", \"capsule\"} }),\n"
            "    density         = 1.0,\n"
            "    friction        = 0.3,\n"
            "    restitution     = 0.1,\n"
            "    gravityScale    = 1.0,\n"
            "    linearDamping   = 0.0,\n"
            "    angularDamping  = 0.0,\n"
            "    fixedRotation   = false,\n"
            "    linearVelocity  = Vector2(0.0),\n"
            "    angularVelocity = 0.0,\n"
            "    _body           = Field(nil, { hidden = true })\n"
            "}\n"
        },
        // UI categories
        {
            "UIText",
            "return Category {\n"
            "    position  = Vector2(0.0),\n"
            "    text      = \"\",\n"
            "    fontSize  = 16.0,\n"
            "    colour    = Field(Vector4(1.0), {isColour = true}),\n"
            "    anchor    = Vector2(0.0),\n"
            "    visible   = true\n"
            "}\n"
        },
        {
            "UIButton",
            "return Category {\n"
            "    position  = Vector2(0.0),\n"
            "    label     = \"Button\",\n"
            "    width     = 120.0,\n"
            "    height    = 40.0,\n"
            "    colour    = Field(Vector4(1.0), {isColour = true}),\n"
            "    hover     = Field(Vector4(1.0), {isColour = true}),\n"
            "    anchor    = Vector2(0.0),\n"
            "    pressed   = Field(false, { hidden = true }),\n"
            "    hovered   = Field(false, { hidden = true }),\n"
            "    visible   = true\n"
            "}\n"
        },
        {
            "UIPanel",
            "return Category {\n"
            "    position  = Vector2(0.0),\n"
            "    width     = 200.0,\n"
            "    height    = 150.0,\n"
            "    colour    = Field(Vector4(0.12, 0.12, 0.16, 0.9), {isColour = true}),\n"
            "    anchor    = Vector2(0.0),\n"
            "    visible   = true\n"
            "}\n"
        }
    };
    return l_entries;
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
    std::filesystem::path l_dir(RanakEngine::Asset::GetTempDir() / "Categories");

    if (!std::filesystem::exists(l_dir))
    {
        std::filesystem::create_directories(l_dir);
    }

    for (const auto& l_entry : GetEntries())
    {
        std::filesystem::path l_path = l_dir / (l_entry.name + ".lua");

        RE::Asset::CreateIfNotExists(l_path.string(), l_entry.source.c_str());

        auto l_file = _contents.resources->Load<RE::Asset::LuaFile>(l_path.string());
        _contents.core->GetLuaContext()->CreateCategory(l_file);
    }
}
