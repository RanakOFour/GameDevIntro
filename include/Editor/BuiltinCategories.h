#ifndef BUILTIN_CATEGORIES_H
#define BUILTIN_CATEGORIES_H

#include "RanakEngine/RanakEngine.h"

#include <string>
#include <vector>

/**
 * @class BuiltinCategories
 * @brief Manages the engine's read-only built-in categories (Transform, Model,
 *        Texture, Shader, PhysicsBody).
 *
 * Category source files are stored in a platform-specific application-data
 * directory so they persist independently of any project and are never mixed
 * with user-defined categories:
 *
 *   Linux :  $XDG_DATA_HOME/GameDevIntro/Categories/   (defaults to ~/.local/share/)
 *   Windows: %APPDATA%\GameDevIntro\Categories\
 *
 * The files are written once on first run and then only regenerated if they
 * are missing.  Because they live outside the project the CategoryPanel must
 * treat them as read-only.
 */
class BuiltinCategories
{
    private:
    struct Entry
    {
        const std::string name;
        const std::string source; ///< Lua source for the category file.
    };

    static const std::vector<Entry>& GetEntries();
    
    public:
    BuiltinCategories() = delete;

    /**
     * @brief Ensures all built-in category files are present in the data
     *        directory, then loads and registers them with the engine.
     *
     * Call this after the engine has been fully initialised (LuaContext must
     * be ready) and before any scene or project is opened.
     *
     * @param _contents Fully initialised engine bundle.
     */
    static void Load(RE::EngineContents& _contents);

    /**
     * @brief Returns the platform-specific data directory used for built-in
     *        category files (e.g. ~/.local/share/GameDevIntro/Categories/).
     */
    static std::string GetDataDir();

    /**
     * @brief Returns true if @p _name is the name of a built-in category.
     *
     * Used by the CategoryPanel to suppress edit controls for these entries.
     */
    static bool IsBuiltin(const std::string& _name);
};

#endif
