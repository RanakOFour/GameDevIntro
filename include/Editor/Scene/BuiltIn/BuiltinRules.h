#ifndef BUILTIN_RULES_H
#define BUILTIN_RULES_H

#include "RanakEngine/RanakEngine.h"

#include <string>
#include <vector>

/**
 * @class BuiltinRules
 * @brief Manages the engine's read-only built-in rules (EditorRender, Rendering, PhysicsSync).
 *
 * Rule source files are stored in the same platform-specific application-data
 * directory as built-in categories so they persist independently of any project:
 *
 *   Linux :  $XDG_DATA_HOME/GameDevIntro/Rules/
 *   Windows: %APPDATA%\GameDevIntro\Rules\
 *
 * Files are written once on first run and only regenerated when missing.
 * The RulesPanel treats them as non-removable but still toggleable (active/inactive).
 * Built-in rules are excluded from scene serialization.
 */
class BuiltinRules
{
    private:
    struct Entry
    {
        const std::string name;
        const std::string source; ///< Lua source for the rule file.
    };

    public:

    BuiltinRules() = delete;

    static const std::vector<Entry>& GetEntries();

    /**
     * @brief Ensures all built-in rule files are present in the data directory,
     *        then loads and registers them with the scene.
     *
     * @param _contents Fully initialised engine bundle.
     */
    static void Load(RE::EngineContents& _contents);

    /**
     * @brief Returns true if @p _name is the name of a built-in rule.
     *
     * Used by the RulesPanel to suppress remove controls and by the serializer
     * to skip these rules during scene save.
     */
    static bool IsBuiltin(const std::string& _name);
};

#endif
