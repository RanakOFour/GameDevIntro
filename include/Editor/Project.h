#ifndef PROJECT_H
#define PROJECT_H

#include <filesystem>
#include <string>

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
