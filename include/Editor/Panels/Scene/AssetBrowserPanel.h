#ifndef ASSET_BROWSER_PANEL_H
#define ASSET_BROWSER_PANEL_H

#include "../../UI/Panel.h"

#include <string>
#include <filesystem>
#include <unordered_map>

#include <GL/glew.h>

class Editor;

class AssetBrowserPanel : public Panel
{
    private:
    std::string m_rootPath;

    GLuint m_folderIcon = 0;
    GLuint m_fileIcon   = 0;
    std::unordered_map<std::string, GLuint> m_thumbnailCache;

    GLuint LoadTextureFromFile(const std::string& _path);
    GLuint GetOrLoadThumbnail(const std::string& _path);

    void DrawDirectoryTree(const std::filesystem::path& _path);
    void Draw() override;

    public:
    AssetBrowserPanel(Editor& _editor);
    ~AssetBrowserPanel();

    void SetRootPath(const std::string& _path) { m_rootPath = _path; }

    void LoadIcons(const std::string& _folderIconPath, const std::string& _fileIconPath);

    static constexpr const char* k_DragDropTexture = "ASSET_TEXTURE_PATH";
    static constexpr const char* k_DragDropModel   = "ASSET_MODEL_PATH";
};

#endif
