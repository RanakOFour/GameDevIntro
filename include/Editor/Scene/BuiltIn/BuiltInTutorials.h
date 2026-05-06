#ifndef BUILTINTUTORIALS_H
#define BUILTINTUTORIALS_H

#include "RanakEngine/RanakEngine.h"

#include <string>
#include <vector>

class BuiltinTutorials
{
    private:
    struct Entry
    {
        const std::string name;
        const std::string source; ///< Lua source for the tutorial scene file.
    };

    std::vector<Entry> GetEntries() const;

    public:
    BuiltinTutorials() = delete;

    /**
     * @brief Loads the built-in tutorial scenes into the engine.
     *
     * Like the built-in categories, these are stored in a platform-specific
     * application-data directory so they persist independently of any project and
     * are never mixed with user-defined scenes:
     *
     *   Linux :  $XDG_DATA_HOME/GameDevIntro/Scenes/   (defaults to ~/.local/share/)
     *   Windows: %APPDATA%\GameDevIntro\Scenes\
     *
     * Call this after the engine has been fully initialised (LuaContext must be
     * ready) and before any project is opened.
     *
     * @param _contents Fully initialised engine bundle.
     */
    static void Load(RE::EngineContents& _contents);
};

#endif