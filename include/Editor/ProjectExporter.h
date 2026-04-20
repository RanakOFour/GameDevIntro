#ifndef PROJECT_EXPORTER_H
#define PROJECT_EXPORTER_H

#include <string>

class Editor;

/**
 * @class ProjectExporter
 * @brief Exports the current project as a standalone executable package.
 *
 * Copies the GamePlayer binary, the active scene, project assets, and
 * engine resources into a self-contained directory that can be distributed.
 * Works on both Linux and Windows.
 */
class ProjectExporter
{
public:
    ProjectExporter() = delete;

    /**
     * @brief Exports the project to @p _outputDir.
     *
     * The export directory will contain:
     *   - The GamePlayer executable (copied from the build directory)
     *   - PlayerConfig.json pointing to the active scene
     *   - The scene .lua file
     *   - resources/ (shaders, models, fonts)
     *   - Project assets (Categories/, Rules/, Textures/, Audio/, Models/)
     *
     * @param _editor    The editor instance (provides engine, project, scene path).
     * @param _outputDir Absolute path to the directory to export into.
     * @return true on success.
     */
    static bool Export(Editor& _editor, const std::string& _outputDir);
};

#endif
