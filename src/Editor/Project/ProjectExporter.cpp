#include "Editor/Project/ProjectExporter.h"

#include "Editor/Core/Editor.h"
#include "Editor/Tabs/SceneEditTab.h"
#include "Editor/Scene/SceneSerializer.h"

#include "RanakEngine/Log.h"

#include "json/json.hpp"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using json = nlohmann::json;

/// Helper: recursively copy a directory (skip if source missing).
static void CopyDirIfExists(const fs::path& _src, const fs::path& _dst)
{
    if (!fs::is_directory(_src))
        return;
    fs::create_directories(_dst);
    fs::copy(_src, _dst, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
}

bool ProjectExporter::Export(Editor& _editor, const std::string& _outputDir)
{
    // Validate prerequisites
    const std::string& l_scenePath = _editor.GetCurrentScenePath();
    if (l_scenePath.empty())
    {
        RE::Log::Message("Export failed: no scene is currently loaded.  Save a scene first.");
        return false;
    }

    if (!_editor.GetProject().IsOpen())
    {
        RE::Log::Message("Export failed: no project is open.");
        return false;
    }

    // Locate the GamePlayer binary next to the editor binary
    // The CMake build puts both targets in the same build directory.
    fs::path l_editorBin = fs::canonical("/proc/self/exe"); // Linux
    fs::path l_buildDir  = l_editorBin.parent_path();

#ifdef _WIN32
    fs::path l_playerBin = l_buildDir / "GamePlayer.exe";
#else
    fs::path l_playerBin = l_buildDir / "GamePlayer";
#endif

    if (!fs::is_regular_file(l_playerBin))
    {
        RE::Log::Message("Export failed: GamePlayer binary not found at "
                         + l_playerBin.string()
                         + "\nBuild the GamePlayer target first.");
        return false;
    }

    // Create output directory
    fs::path l_outDir(_outputDir);
    fs::create_directories(l_outDir);

    // Save the current scene before exporting
    SceneSerializer::SaveToFile(l_scenePath, _editor.GetEngineContents(),
                                _editor.GetSceneEdit().GetSceneSettings());

    // Copy the scene file
    fs::path l_sceneFile = fs::path(l_scenePath).filename();
    fs::copy_file(l_scenePath, l_outDir / l_sceneFile,
                  fs::copy_options::overwrite_existing);

    // Copy player binary
    fs::path l_dstPlayer = l_outDir / l_playerBin.filename();
    fs::copy_file(l_playerBin, l_dstPlayer,
                  fs::copy_options::overwrite_existing);

#ifndef _WIN32
    // Ensure the binary is executable.
    fs::permissions(l_dstPlayer, fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
                    fs::perm_options::add);
#endif

    // Write PlayerConfig.json
    {
        json l_config;
        l_config["scene"]        = l_sceneFile.string();
        l_config["windowWidth"]  = 1920;
        l_config["windowHeight"] = 1080;
        l_config["title"]        = fs::path(_editor.GetProject().GetRootPath()).filename().string();

        std::ofstream l_f(l_outDir / "PlayerConfig.json");
        l_f << l_config.dump(4);
    }

    // Copy engine resources (shaders, models, fonts)
    std::filesystem::path l_tempDir = RE::Asset::GetTempDir();
    CopyDirIfExists(l_tempDir / "Shaders", l_outDir / "resources" / "Shaders");
    CopyDirIfExists(l_tempDir / "Models",  l_outDir / "resources" / "Models");
    CopyDirIfExists(l_tempDir / "Fonts",   l_outDir / "resources" / "Fonts");

    // Copy project asset directories
    const Project& l_proj = _editor.GetProject();
    CopyDirIfExists(l_proj.GetCategoriesDir(), l_outDir / "Categories");
    CopyDirIfExists(l_proj.GetRulesDir(),      l_outDir / "Rules");
    CopyDirIfExists(l_proj.GetTexturesDir(),   l_outDir / "Textures");
    CopyDirIfExists(l_proj.GetAudioDir(),      l_outDir / "Audio");
    CopyDirIfExists(l_proj.GetModelsDir(),     l_outDir / "Models");

#ifdef _WIN32
    // Copy DLLs needed at runtime on Windows.
    CopyDirIfExists(fs::path("lib") / "win" / "bin", l_outDir);
#endif

    RE::Log::Message("Project exported to: " + l_outDir.string());
    return true;
}
