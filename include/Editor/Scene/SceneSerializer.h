#ifndef SCENE_SERIALIZER_H
#define SCENE_SERIALIZER_H

#include "RanakEngine/RanakEngine.h"
#include "./SceneSettings.h"

#include <string>

/**
 * @class SceneSerializer
 * @brief Static class for saving and loading scenes as fully self-contained Lua scripts.
 *
 * Serialize() converts a live Scene into a portable Lua script that embeds the
 * full source code of every category and rule used by the scene.
 * The resulting script can recreate the scene on any machine even if the original 
 * .lua asset files are absent, though any resources loaded by the rules (e.g. sprites) must still be present.
 *
 * Generated script structure:
 *  1. Category definitions – each category's Lua source is embedded verbatim
 *     inside a call to `LoadCategory(name, [[...]])`.
 *  2. Rule definitions     – each rule's Lua source is embedded inside a call
 *     to `LoadRule(name, [[...]])`.
 *  3. Entity reconstruction – every entity is recreated, added to its
 *     categories, and has all per-entity field values restored.
 *
 * SaveToFile()     – convenience wrapper that calls Serialize() and writes
 *                    the result to a file.
 * LoadFromFile()   – reads the script from disk and executes it.
 * LoadFromString() – executes a script that is already in memory.
 *
 * Before executing a scene script, LoadFromFile / LoadFromString register the
 * C++ helper Lua globals `LoadCategory` and `LoadRule`, and
 * expose the active Scene as the global `scene`.
 */
class SceneSerializer
{
    private:
    /**
     * @brief Converts a sol::object field value to a Lua literal string.
     *
     * Handles numbers, booleans, strings, Vector2, Vector3, and Vector4 userdata.
     *
     * @param _value The sol object to convert.
     * @return Lua source literal suitable for direct embedding in a script.
     */
    static std::string SolObjectToLua(const sol::object& _value);

    /**
     * @brief Generates a `return Category { ... }` Lua source string from a
     *        category's base-fields table.
     *
     * Used as a fallback when the category has no origin LuaFile.
     *
     * @param _category The category whose base fields should be serialised.
     * @return Lua source that reconstructs the category's field schema.
     */
    static std::string GenerateCategoryCode(RE::Core::Category& _category);

    /**
     * @brief Registers `LoadCategory(name, code)` and
     *        `LoadRule(name, code)` as Lua globals.
     *
     * Each helper writes the embedded code to a uniquely-named temporary file,
     * loads it through the AssetManager (so the LuaFile acquires the correct
     * name), calls the appropriate LuaContext factory, then removes the temp
     * file.  Using a unique per-call suffix prevents stale AssetManager cache
     * hits across multiple scene loads.
     *
     * Called automatically by LoadFromFile() / LoadFromString().
     *
     * @param _contents Engine contents bundle.
     */
    static void RegisterConstructorHelpers(RE::EngineContents& _contents,
                                           SceneSettings* _outSettings);

    public:
    SceneSerializer() = delete;

    /**
     * @brief Converts a live scene to a self-constructing, fully portable Lua script.
     *
     * Categories are embedded using their origin-file source (via
     * LuaFile::GetCode()).  If no origin file is recorded the source is
     * reconstructed from the category's base-fields table.  Rules are embedded
     * using the source cached in the AssetManager under the conventional path
     * `./resources/Rules/<name>.lua`; if that file cannot be found a warning
     * comment is emitted instead.
     *
     * @param _scene    The scene to serialise.
     * @param _contents Engine contents (provides LuaContext + AssetManager).
     * @return          Lua source code as a std::string.
     */
    /**
     * @param _settings  Scene settings (gravity, clear colour) to embed in the script.
     */
    static std::string Serialize(RE::EngineContents& _contents,
                                 const SceneSettings& _settings);

    /**
     * @brief Serializes the scene and writes the resulting Lua script to a file.
     *
     * @param _filePath Destination file path (created / overwritten).
     * @param _contents Engine contents (provides LuaContext + AssetManager).
     * @param _settings Scene settings to embed.
     */
    static void SaveToFile(const std::string& _filePath,
                           RE::EngineContents& _contents,
                           const SceneSettings& _settings);

    /**
     * @brief Reconstructs a scene by executing a Lua script file on disk.
     *
     * @param _filePath   Path to the previously saved scene script.
     * @param _contents   Engine contents bundle.
     * @param _outSettings If non-null, receives the settings read from the script.
     */
    static void LoadFromFile(const std::string& _filePath,
                             RE::EngineContents& _contents,
                             SceneSettings* _outSettings = nullptr);

    /**
     * @brief Reconstructs a scene by executing a Lua script provided as a string.
     *
     * @param _script      Lua source code (as produced by Serialize()).
     * @param _contents    Engine contents bundle.
     * @param _outSettings If non-null, receives the settings read from the script.
     */
    static void LoadFromString(const std::string& _script,
                               RE::EngineContents& _contents,
                               SceneSettings* _outSettings = nullptr);
};

#endif // SCENE_SERIALIZER_H
