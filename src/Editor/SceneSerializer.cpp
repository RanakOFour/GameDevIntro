#include "Editor/SceneSerializer.h"

#include "RanakEngine/Core/Scene.h"
#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Core/EntityRegistry.h"
#include "RanakEngine/Core/Rule.h"
#include "RanakEngine/Core/Category.h"
#include "RanakEngine/Asset/LuaFile.h"
#include "RanakEngine/Math/Vector2.h"
#include "RanakEngine/Math/Vector3.h"
#include "RanakEngine/Math/Vector4.h"

#include "sol/sol.hpp"

#include <sstream>
#include <fstream>
#include <filesystem>
#include <set>
#include <iomanip>

namespace fs = std::filesystem;

std::string SceneSerializer::SolObjectToLua(const sol::object& _value)
{
    switch (_value.get_type())
    {
    case sol::type::number:
    {
        if (_value.is<int>())
            return std::to_string(_value.as<int>());

        std::ostringstream oss;
        oss << std::setprecision(7) << _value.as<double>();
        std::string s = oss.str();
        if (s.find('.') == std::string::npos && s.find('e') == std::string::npos)
            s += ".0";
        return s;
    }

    case sol::type::boolean:
        return _value.as<bool>() ? "true" : "false";

    case sol::type::string:
    {
        std::string raw = _value.as<std::string>();
        std::string escaped;
        escaped.reserve(raw.size() + 2);
        for (char c : raw)
        {
            if      (c == '\\') escaped += "\\\\";
            else if (c == '"')  escaped += "\\\"";
            else                escaped += c;
        }
        return "\"" + escaped + "\"";
    }

    case sol::type::userdata:
        if (_value.is<Vector4>())
        {
            Vector4 v = _value.as<Vector4>();
            std::ostringstream oss;
            oss << std::setprecision(7)
                << "Vector4(" << v.x << ", " << v.y
                << ", " << v.z << ", " << v.w << ")";
            return oss.str();
        }
        if (_value.is<Vector3>())
        {
            Vector3 v = _value.as<Vector3>();
            std::ostringstream oss;
            oss << std::setprecision(7)
                << "Vector3(" << v.x << ", " << v.y << ", " << v.z << ")";
            return oss.str();
        }
        if (_value.is<Vector2>())
        {
            Vector2 v = _value.as<Vector2>();
            std::ostringstream oss;
            oss << std::setprecision(7)
                << "Vector2(" << v.x << ", " << v.y << ")";
            return oss.str();
        }
        return "nil -- unsupported userdata";

    case sol::type::nil:
        return "nil";

    default:
        return "nil -- unsupported type";
    }
}

std::string SceneSerializer::GenerateCategoryCode(RE::Core::Category& _category)
{
    std::ostringstream out;
    out << "return Category {\n";
    out << "    name = " << _category.GetName().c_str() << ",\n";

    sol::table fields = _category.GetBaseData();
    for (auto& pair : fields.pairs())
    {
        if (pair.first.get_type() != sol::type::string)
            continue;
        out << "    " << pair.first.as<std::string>()
            << " = " << SolObjectToLua(pair.second) << ",\n";
    }
    out << "}\n";
    return out.str();
}

void SceneSerializer::RegisterConstructorHelpers(RE::EngineContents& _contents)
{
    sol::state* l_state = _contents.core->GetLuaContext()->GetState();

    // LoadCategory(name, code)
    // Writes the embedded source to a temp file named exactly `name.lua` so
    // that LuaFile::GetName() returns `name` unchanged.  LuaContext::CreateCategory
    // derives the category name from the filename stem, so the stem MUST equal
    // the intended category name.
    (*l_state)["LoadCategory"] =
        [&_contents](const std::string& _name, const std::string& _code)
    {
        // Use the category name as the filename stem so GetName() returns the
        // correct name without any prefix/suffix mangling.
        fs::path l_tmpPath = fs::temp_directory_path() / (_name + ".lua");

        {
            std::ofstream l_out(l_tmpPath);
            l_out << _code;
        }

        auto l_file = _contents.resources->Load<RE::Asset::LuaFile>(l_tmpPath.string());
        _contents.core->GetLuaContext()->CreateCategory(l_file);

        fs::remove(l_tmpPath);

        RE::Log::Message("[SceneSerializer] Embedded category registered: " + _name);
    };

    // LoadRule(name, code)
    // Same pattern: temp file named exactly `name.lua` -> CreateRule(file) -> AddRule.
    (*l_state)["LoadRule"] =
        [&_contents](const std::string& _name, const std::string& _code)
    {
        fs::path l_tmpPath = fs::temp_directory_path() / (_name + ".lua");

        {
            std::ofstream l_out(l_tmpPath);
            l_out << _code;
        }

        auto l_file = _contents.resources->Load<RE::Asset::LuaFile>(l_tmpPath.string());
        RE::Core::Rule l_rule = _contents.core->GetLuaContext()->CreateRule(l_file);
        _contents.core->GetScene().lock()->AddRule(l_rule);

        fs::remove(l_tmpPath);

        RE::Log::Message("[SceneSerializer] Embedded rule registered: " + _name);
    };
}

std::string SceneSerializer::Serialize(RE::EngineContents& _contents)
{
    std::shared_ptr<RE::Core::Scene> l_scenePtr = _contents.core->GetScene().lock();
    if (!l_scenePtr)
    {
        RE::Log::Message("[SceneSerializer] ERROR: No active scene to serialize.");
        return "";
    }

    auto l_context = _contents.core->GetLuaContext();

    std::ostringstream out;
    out << "-- ========================================\n";
    out << "-- Scene: " << l_scenePtr->GetName() << "\n";
    out << "-- Generated by SceneSerializer\n";
    out << "-- ========================================\n\n";

    // Collect all category names referenced by live entities.
    std::set<std::string> l_usedCategories;
    sol::table l_entityTable = l_scenePtr->GetRegistry().GetEntityTable();

    for (auto& l_entityPair : l_entityTable.pairs())
    {
        int l_id = l_entityPair.first.as<int>();
        sol::table l_attributes = l_scenePtr->GetRegistry().GetEntityAttributes(l_id);
        for (auto& l_catPair : l_attributes.pairs())
        {
            if (l_catPair.first.get_type() == sol::type::string)
                l_usedCategories.insert(l_catPair.first.as<std::string>());
        }
    }

    // Embed category source code.
    out << "-- ========================================\n";
    out << "-- CATEGORIES\n";
    out << "-- ========================================\n\n";

    for (const auto& catName : l_usedCategories)
    {
        auto l_category = l_context->GetCategory(catName).lock();
        if (!l_category)
        {
            out << "-- WARNING: category \"" << catName
                << "\" not found in LuaContext; skipped.\n\n";
            continue;
        }

        // Prefer the actual source file; fall back to construction from fields.
        std::string l_code;
        auto l_originFile = l_category->GetOriginFile().lock();
        if (l_originFile)
        {
            l_code = l_originFile->GetCode();
        }
        else
        {
            RE::Log::Message("[SceneSerializer] No origin file for category \""
                             + catName + "\"; generating from base fields.");
            l_code = GenerateCategoryCode(*l_category);
        }

        out << "-- Category: " << catName << "\n";
        out << "print(\"[Scene] Defining category: " << catName << "\")\n";
        // Lua long-bracket strings: no escaping needed for embedded Lua code.
        out << "LoadCategory(\"" << catName << "\", [[\n";
        out << l_code;
        out << "]])\n\n";
    }

    // ------------------------------------------------------------------
    // 3. Embed rule source code.
    // ------------------------------------------------------------------
    out << "-- ========================================\n";
    out << "-- RULES\n";
    out << "-- ========================================\n\n";

    {
        sol::table l_sceneTable = l_scenePtr->GetSceneTable();
        sol::optional<sol::table> l_rulesOpt =
            l_sceneTable.get<sol::optional<sol::table>>("Rules");

        if (l_rulesOpt)
        {
            for (auto& rulePair : l_rulesOpt->pairs())
            {
                if (rulePair.first.get_type() != sol::type::string)
                    continue;

                std::string l_ruleName = rulePair.first.as<std::string>();

                // The EditorRender rule is added by SceneEditTab; skip it.
                if (l_ruleName == "EditorRender")
                    continue;

                // Retrieve the rule shared_ptr from the scene table and use
                // its origin file directly now that Rule stores one.
                std::shared_ptr<RE::Core::Rule> l_rulePtr =
                    l_rulesOpt->raw_get<std::shared_ptr<RE::Core::Rule>>(l_ruleName);

                auto l_file = l_rulePtr ? l_rulePtr->GetOriginFile().lock()
                                        : std::shared_ptr<RE::Asset::LuaFile>{};

                if (!l_file)
                {
                    out << "-- WARNING: no origin file recorded for rule \""
                        << l_ruleName << "\"; rule skipped.\n\n";
                    RE::Log::Message("[SceneSerializer] WARNING: rule \""
                                     + l_ruleName + "\" has no origin file.");
                    continue;
                }

                out << "-- Rule: " << l_ruleName << "\n";
                out << "print(\"[Scene] Defining rule: " << l_ruleName << "\")\n";
                out << "LoadRule(\"" << l_ruleName << "\", [[\n";
                out << l_file->GetCode();
                out << "]])\n\n";
            }
        }
    }

    // Recreate entities and restore attribute values.
    out << "-- ========================================\n";
    out << "-- ENTITIES\n";
    out << "-- ========================================\n\n";

    for (auto& l_entityPair : l_entityTable.pairs())
    {
        int l_id = l_entityPair.first.as<int>();
        std::string l_eVar = "e" + std::to_string(l_id);
        std::string l_entityName = l_scenePtr->GetRegistry().GetEntityName(l_id);

        out << "-- " << l_entityName << " (id=" << l_id << ")\n";
        out << "print(\"[Scene] Creating entity: " << l_entityName << "\")\n";
        out << "local " << l_eVar << " = scene:addEntity()\n";

        sol::table l_attrs = l_scenePtr->GetRegistry().GetEntityAttributes(l_id);

        for (auto& l_catPair : l_attrs.pairs())
        {
            if (l_catPair.first.get_type() != sol::type::string)
                continue;

            std::string l_catName  = l_catPair.first.as<std::string>();
            sol::table l_catFields = l_catPair.second.as<sol::table>();

            out << "scene:addToCategory(" << l_eVar << ", \"" << l_catName << "\")\n";

            std::string l_attrVar = l_eVar + "_" + l_catName;
            out << "local " << l_attrVar
                << " = scene:getAttributesOf(" << l_eVar
                << ")[\"" << l_catName << "\"]\n";
            out << "if " << l_attrVar << " then\n";

            for (auto& l_fieldPair : l_catFields.pairs())
            {
                if (l_fieldPair.first.get_type() != sol::type::string)
                    continue;

                // Skip nil values — the category default already supplies them via AddMember.
                // Skip unsupported userdata (Model, Shader, Texture etc.) — they cannot be
                // round-tripped; the category default or a rule is responsible for re-loading them.
                sol::type l_valType = l_fieldPair.second.get_type();
                if (l_valType == sol::type::nil)
                    continue;
                if (l_valType == sol::type::userdata &&
                    !l_fieldPair.second.is<Vector2>() &&
                    !l_fieldPair.second.is<Vector3>() &&
                    !l_fieldPair.second.is<Vector4>())
                    continue;

                out << "    " << l_attrVar << "[\"" << l_fieldPair.first.as<std::string>()
                    << "\"] = " << SolObjectToLua(l_fieldPair.second) << "\n";
            }
            out << "end\n\n";
        }
    }

    out << "print(\"[Scene] Scene '" << l_scenePtr->GetName()
        << "' constructed successfully.\")\n";

    return out.str();
}


void SceneSerializer::SaveToFile(const std::string& _filePath,
                                 RE::EngineContents& _contents)
{
    std::shared_ptr<RE::Core::Scene> l_scenePtr = _contents.core->GetScene().lock();
    if (!l_scenePtr)
    {
        RE::Log::Message("[SceneSerializer] ERROR: No active scene to save.");
        return;
    }

    std::string l_script = Serialize(_contents);

    std::ofstream l_file;
    l_file.open(_filePath, std::ios::out | std::ios::trunc);

    if(!l_file)
    {
        RE::Log::Message("[SceneSerializer] Could not open file for writing: " + _filePath + "\nAttempting to create new file.");
        l_file.clear();
        l_file.open(_filePath, std::ios::out);
    }

    if (!l_file.is_open())
    {
        RE::Log::Message("[SceneSerializer] ERROR: Could not open file for writing: "
                         + _filePath);
        return;
    }

    l_file << l_script;
    l_file.close();

    RE::Log::Message("[SceneSerializer] Scene saved to: " + _filePath);
}


void SceneSerializer::LoadFromFile(const std::string& _filePath,
                                   RE::EngineContents& _contents)
{
    std::ifstream l_file(_filePath);
    if (!l_file.is_open())
    {
        RE::Log::Message("[SceneSerializer] ERROR: Could not open scene file: "
                         + _filePath);
        return;
    }

    std::string l_script((std::istreambuf_iterator<char>(l_file)),
                           std::istreambuf_iterator<char>());
    l_file.close();

    RE::Log::Message("[SceneSerializer] Loading scene from: " + _filePath);
    LoadFromString(l_script, _contents);
}

void SceneSerializer::LoadFromString(const std::string& _script,
                                     RE::EngineContents& _contents)
{
    sol::state* l_state = _contents.core->GetLuaContext()->GetState();

    // Register C++ helpers that the embedded script will call.
    RegisterConstructorHelpers(_contents);

    // Expose the active scene so entities can be created and populated.
    auto l_scene = _contents.core->GetScene().lock();
    if (!l_scene)
    {
        RE::Log::Message("[SceneSerializer] ERROR: No active scene to construct into.");
        return;
    }
    (*l_state)["scene"] = l_scene.get();

    auto l_result = l_state->safe_script(_script, sol::script_pass_on_error);
    if (!l_result.valid())
    {
        sol::error l_err = l_result;
        RE::Log::Message(std::string("[SceneSerializer] ERROR running scene script:\n")
                         + l_err.what());
    }
}
