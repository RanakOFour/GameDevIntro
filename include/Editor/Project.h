#ifndef PROJECT_H
#define PROJECT_H

#include <filesystem>
#include <string>

/**
 * @struct ProjectSettings
 * @brief Persistent per-project configuration saved inside ProjectInfo.json.
 *
 * Groups engine-level settings that are constant for a project but meaningless
 * to embed in individual scene files (e.g. physics gravity, background colour).
 * All fields have sensible defaults so a missing JSON block is harmless.
 */
struct ProjectSettings
{
    // --- Camera (initial state; applied when a fresh scene is loaded) ---
    float cameraX     =  0.0f;  ///< Initial camera world X position.
    float cameraY     =  0.0f;  ///< Initial camera world Y position.
    float cameraZ     = 10.0f;  ///< Initial camera world Z position.
    float cameraWidth = 30.0f;  ///< Initial orthographic half-width (zoom).
    bool  cameraPerspective = false; ///< True = perspective, false = orthographic.
};

/**
 * @class Project
 * @brief Represents an open project directory that contains the user's categories,
 *        rules, scenes, and assets.
 *
 * Engine assets (shaders, editor Lua, fonts, tutorials) remain under ./resources/
 * and are unaffected by the project path.
 */
class Project
{
    private:
    std::string m_rootPath;
    ProjectSettings m_settings;
    
    public:
    Project() = default;
    Project(const std::string& _path) : m_rootPath(_path) {}

    /** @brief The project root directory. */
    const std::string& GetRootPath() const { return m_rootPath; }

    /** @brief Returns e.g. <root>/Categories */
    std::string GetCategoriesDir() const { return (std::filesystem::path(m_rootPath) / "Categories").string(); }
    /** @brief Returns e.g. <root>/Rules */
    std::string GetRulesDir()      const { return (std::filesystem::path(m_rootPath) / "Rules").string(); }
    /** @brief Returns e.g. <root>/Scenes */
    std::string GetScenesDir()     const { return (std::filesystem::path(m_rootPath) / "Scenes").string(); }
    /** @brief Returns e.g. <root>/Textures */
    std::string GetTexturesDir()   const { return (std::filesystem::path(m_rootPath) / "Textures").string(); }
    /** @brief Returns e.g. <root>/Audio */
    std::string GetAudioDir()      const { return (std::filesystem::path(m_rootPath) / "Audio").string(); }
    /** @brief Returns e.g. <root>/Models */
    std::string GetModelsDir()     const { return (std::filesystem::path(m_rootPath) / "Models").string(); }
    /** @brief Returns e.g. <root>/ProjectInfo.json */
    std::string GetProjectInfoPath() const { return (std::filesystem::path(m_rootPath) / "ProjectInfo.json").string(); }

    /** @brief Returns a mutable reference to the project's persistent settings. */
    ProjectSettings& GetSettings() { return m_settings; }
    /** @brief Returns a const reference to the project's persistent settings. */
    const ProjectSettings& GetSettings() const { return m_settings; }

    /** @brief Returns true if this Project holds a non-empty path. */
    bool IsOpen() const { return !m_rootPath.empty(); }

    /**
     * @brief Creates the standard project subdirectory structure on disk.
     *
     * Creates the root directory and all subdirectories.  Existing directories
     * are left untouched.
     *
     * @param _rootPath Absolute or relative path for the project root.
     * @return A Project initialised to the given path.
     */
    static Project Create(const std::string& _rootPath);

    /**
     * @brief Checks whether a directory looks like a valid project.
     *
     * Considers valid if at least one of the expected subdirectories exists.
     *
     * @param _rootPath Path to test.
     * @return true if the directory appears to be a project root.
     */
    static bool IsValid(const std::string& _rootPath);
};

#endif
