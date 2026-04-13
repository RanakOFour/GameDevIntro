#include "Editor/SceneSerializer.h"
#include "Editor/SceneSettings.h"
#include "Editor/BuiltinRules.h"

#include "RanakEngine/Core/Scene.h"
#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Core/EntityRegistry.h"
#include "RanakEngine/Core/Rule.h"
#include "RanakEngine/Core/Category.h"
#include "RanakEngine/Asset/LuaFile.h"
#include "RanakEngine/Math.h"

#include "sol/sol.hpp"

#include <sstream>
#include <fstream>
#include <filesystem>
#include <set>
#include <iomanip>

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

void SceneSerializer::RegisterConstructorHelpers(RE::EngineContents& _contents,
                                                  SceneSettings* _outSettings)
{
    sol::state* l_state = _contents.core->GetLuaContext()->GetState();

    // SetSceneSettings(gravityX, gravityY, r, g, b, a)
    // Called at the top of a scene script to restore simulation/render settings.
    // Writes into *_outSettings when provided; harmless no-op otherwise.
    (*l_state)["SetSceneSettings"] =
        [_outSettings](float _gx, float _gy, float _r, float _g, float _b, float _a)
    {
        if (_outSettings)
        {
            _outSettings->gravityX    = _gx;
            _outSettings->gravityY    = _gy;
            _outSettings->clearColorR = _r;
            _outSettings->clearColorG = _g;
            _outSettings->clearColorB = _b;
            _outSettings->clearColorA = _a;
        }
    };
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
        std::filesystem::path l_tmpPath = std::filesystem::temp_directory_path() / (_name + ".lua");

        {
            std::ofstream l_out(l_tmpPath);
            l_out << _code;
        }

        auto l_file = _contents.resources->Load<RE::Asset::LuaFile>(l_tmpPath.string());
        _contents.core->GetLuaContext()->CreateCategory(l_file);

        std::filesystem::remove(l_tmpPath);

        RE::Log::Message("Category registered: " + _name);
    };

    // LoadRule(name, code)
    // Same pattern: temp file named exactly `name.lua` -> CreateRule(file) -> AddRule.
    (*l_state)["LoadRule"] =
        [&_contents](const std::string& _name, const std::string& _code)
    {
        std::filesystem::path l_tmpPath = std::filesystem::temp_directory_path() / (_name + ".lua");

        {
            std::ofstream l_out(l_tmpPath);
            l_out << _code;
        }

        auto l_file = _contents.resources->Load<RE::Asset::LuaFile>(l_tmpPath.string());
        RE::Core::Rule l_rule = _contents.core->GetLuaContext()->CreateRule(l_file);
        _contents.core->GetScene().lock()->AddRule(l_rule);

        std::filesystem::remove(l_tmpPath);

        RE::Log::Message("Embedded rule registered: " + _name);
    };

    // LoadCategoryFromFile(path)
    // Loads a category directly from its source file on disk.
    // If a category with the derived name is already registered, skip creation
    // to avoid a nested Lua execution that may fail.
    (*l_state)["LoadCategoryFromFile"] =
        [&_contents](const std::string& _path)
    {
        auto l_luaContext = _contents.core->GetLuaContext();

        // Derive the expected category name from the filename stem.
        std::string l_name;
        {
            auto l_slash = _path.find_last_of("/\\");
            auto l_dot   = _path.find_last_of('.');
            std::size_t l_start = (l_slash == std::string::npos) ? 0 : l_slash + 1;
            l_name = _path.substr(l_start, (l_dot == std::string::npos || l_dot < l_start)
                                           ? std::string::npos : l_dot - l_start);
        }

        // If already registered, nothing to do.
        if (!l_luaContext->GetCategory(l_name).expired())
        {
            RE::Log::Message("Category already registered, skipping: " + l_name);
            return;
        }

        auto l_file = _contents.resources->Load<RE::Asset::LuaFile>(_path);
        if (l_file.expired())
        {
            RE::Log::Message("ERROR: Could not load category file: " + _path);
            return;
        }
        l_luaContext->CreateCategory(l_file);
        RE::Log::Message("Category loaded from file: " + _path);
    };

    // LoadRuleFromFile(path)
    // Loads a rule directly from its source file on disk.
    (*l_state)["LoadRuleFromFile"] =
        [&_contents](const std::string& _path)
    {
        try
        {
            auto l_file = _contents.resources->Load<RE::Asset::LuaFile>(_path);
            if (l_file.expired())
            {
                RE::Log::Message("ERROR: Could not load rule file: " + _path);
                return;
            }
            RE::Core::Rule l_rule = _contents.core->GetLuaContext()->CreateRule(l_file);
            _contents.core->GetScene().lock()->AddRule(l_rule);
            RE::Log::Message("Rule loaded from file: " + _path);
        }
        catch (const std::exception& e)
        {
            RE::Log::Message("ERROR: Exception loading rule from " + _path + ": " + e.what());
        }
    };
}

std::string SceneSerializer::Serialize(RE::EngineContents& _contents,
                                        const SceneSettings& _settings)
{
    std::shared_ptr<RE::Core::Scene> l_scenePtr = _contents.core->GetScene().lock();
    if (!l_scenePtr)
    {
        RE::Log::Message("ERROR: No active scene to serialize.");
        return "";
    }

    auto l_context = _contents.core->GetLuaContext();
    auto l_entityRegistry = l_scenePtr->GetRegistry();

    std::ostringstream out;
    out << "-- ========================================\n";
    out << "-- Scene: " << l_scenePtr->GetName() << "\n";
    out << "-- Generated by SceneSerializer\n";
    out << "-- ========================================\n\n";

    // Scene settings block — restored first so downstream code can rely on them.
    out << "-- ========================================\n";
    out << "-- SCENE SETTINGS\n";
    out << "-- ========================================\n\n";
    out << std::setprecision(7);
    out << "SetSceneSettings("
        << _settings.gravityX    << ", " << _settings.gravityY    << ", "
        << _settings.clearColorR << ", " << _settings.clearColorG << ", "
        << _settings.clearColorB << ", " << _settings.clearColorA << ")\n\n";

    // Collect all category names referenced by live entities.
    std::set<std::string> l_usedCategories;
    sol::table l_entityTable = l_entityRegistry.GetEntityTable();

    for (auto& l_entityPair : l_entityTable.pairs())
    {
        int l_id = l_entityPair.first.as<int>();
        sol::table l_attributes = l_entityRegistry.GetEntityAttributes(l_id);
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

        auto l_originFile = l_category->GetOriginFile().lock();

        out << "-- Category: " << catName << "\n";
        out << "print(\"[Scene] Defining category: " << catName << "\")\n";

        if (l_originFile && !l_originFile->GetPath().empty())
        {
            // Load directly from the source file on disk.
            std::string l_path = l_originFile->GetPath();
            std::string l_escaped;
            for (char c : l_path)
            {
                if      (c == '\\') l_escaped += "\\\\";
                else if (c == '"')  l_escaped += "\\\"";
                else                l_escaped += c;
            }
            out << "LoadCategoryFromFile(\"" << l_escaped << "\")\n\n";
        }
        else
        {
            // Fall back to inline code.
            std::string l_code;
            if (l_originFile)
            {
                l_code = l_originFile->GetCode();
            }
            else
            {
                RE::Log::Message("No origin file for category \""
                                 + catName + "\"; generating from base fields.");
                l_code = GenerateCategoryCode(*l_category);
            }
            // Lua long-bracket strings: no escaping needed for embedded Lua code.
            out << "LoadCategory(\"" << catName << "\", [[\n";
            out << l_code;
            out << "]])\n\n";
        }
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

                // Skip all built-in rules — they are re-added after load.
                if (BuiltinRules::IsBuiltin(l_ruleName))
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
                    RE::Log::Message("WARNING: rule \""
                                     + l_ruleName + "\" has no origin file.");
                    continue;
                }

                out << "-- Rule: " << l_ruleName << "\n";
                out << "print(\"[Scene] Defining rule: " << l_ruleName << "\")\n";

                std::string l_filePath = l_file->GetPath();
                if (!l_filePath.empty())
                {
                    // Load directly from the source file on disk.
                    std::string l_escaped;
                    for (char c : l_filePath)
                    {
                        if      (c == '\\') l_escaped += "\\\\";
                        else if (c == '"')  l_escaped += "\\\"";
                        else                l_escaped += c;
                    }
                    out << "LoadRuleFromFile(\"" << l_escaped << "\")\n\n";
                }
                else
                {
                    out << "LoadRule(\"" << l_ruleName << "\", [[\n";
                    out << l_file->GetCode();
                    out << "]])\n\n";
                }
            }
        }
    }

    // Recreate entities and restore attribute values.
    out << "-- ========================================\n";
    out << "-- ENTITIES\n";
    out << "-- ========================================\n\n";

    std::vector<int> l_entityIds = l_entityRegistry.GetAllRegisteredIds();

    for (int i = 0; i < l_entityIds.size(); i++)
    {
        int l_id = l_entityIds[i];
        sol::table l_currentEntityTable = l_entityTable.raw_get<sol::table>(l_id);
        std::string l_eVar = "e" + std::to_string(l_id);
        std::string l_entityName = l_entityRegistry.GetEntityName(l_id);

        out << "-- " << l_entityName << " (id=" << l_id << ")\n";
        out << "print(\"[Scene] Creating entity: " << l_entityName << "\")\n";
        out << "local " << l_eVar << " = Scene:addEntity()\n";

        sol::table l_attrs = l_entityRegistry.GetEntityAttributes(l_id);

        for (auto& l_catPair : l_attrs.pairs())
        {
            if (l_catPair.first.get_type() != sol::type::string)
                continue;

            std::string l_catName  = l_catPair.first.as<std::string>();
            sol::table l_catFields = l_catPair.second.as<sol::table>();

            // Transform is automatically added by Scene:addEntity(), so skip the explicit call.
            if (l_catName != "Transform")
                out << "Scene:addToCategory(" << l_eVar << ", \"" << l_catName << "\")\n";

            std::string l_attrVar = l_eVar + "_" + l_catName;
            out << "local " << l_attrVar
                << " = Scene:getAttributesOf(" << l_eVar
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
                                 RE::EngineContents& _contents,
                                 const SceneSettings& _settings)
{
    std::shared_ptr<RE::Core::Scene> l_scenePtr = _contents.core->GetScene().lock();
    if (!l_scenePtr)
    {
        RE::Log::Message("ERROR: No active scene to save.");
        return;
    }

    std::string l_script = Serialize(_contents, _settings);

    std::ofstream l_file;
    l_file.open(_filePath, std::ios::out | std::ios::trunc);

    if(!l_file)
    {
        RE::Log::Message("Could not open file for writing: " + _filePath + "\nAttempting to create new file.");
        l_file.clear();
        l_file.open(_filePath, std::ios::out);
    }

    if (!l_file.is_open())
    {
        RE::Log::Message("ERROR: Could not open file for writing: "
                         + _filePath);
        return;
    }

    l_file << l_script;
    l_file.close();

    RE::Log::Message("Scene saved to: " + _filePath);
}


void SceneSerializer::LoadFromFile(const std::string& _filePath,
                                   RE::EngineContents& _contents,
                                   SceneSettings* _outSettings)
{
    std::ifstream l_file(_filePath);
    if (!l_file.is_open())
    {
        RE::Log::Message("ERROR: Could not open scene file: "
                         + _filePath);
        return;
    }

    std::string l_script((std::istreambuf_iterator<char>(l_file)),
                           std::istreambuf_iterator<char>());
    l_file.close();

    RE::Log::Message("Loading scene from: " + _filePath);
    LoadFromString(l_script, _contents, _outSettings);
}

void SceneSerializer::LoadFromString(const std::string& _script,
                                     RE::EngineContents& _contents,
                                     SceneSettings* _outSettings)
{
    sol::state* l_state = _contents.core->GetLuaContext()->GetState();

    // Register helper functions.
    RegisterConstructorHelpers(_contents, _outSettings);

    // Create new scene
    auto l_newScene = std::make_shared<RE::Core::Scene>();
    _contents.core->SetScene(l_newScene);
    
    if (!l_newScene)
    {
        RE::Log::Message("ERROR: No active scene to construct into.");
        return;
    }

    l_state->set("Scene", l_newScene);

    auto l_result = l_state->safe_script(_script, sol::script_pass_on_error);
    if (!l_result.valid())
    {
        sol::error l_err = l_result;
        RE::Log::Message(std::string("ERROR running scene script:\n")
                         + l_err.what());
    }

    l_state->set("Scene", sol::nil);
}
