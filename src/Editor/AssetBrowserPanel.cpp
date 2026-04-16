#include "Editor/AssetBrowserPanel.h"
#include "Editor/Editor.h"
#include "Editor/SceneEditTab.h"

#include "imgui/imgui.h"

#include "stb_image.h"

#include <algorithm>
#include <vector>

AssetBrowserPanel::AssetBrowserPanel(Editor& _editor)
: Panel("Asset Browser", _editor)
{
}

AssetBrowserPanel::~AssetBrowserPanel()
{
    if (m_folderIcon) glDeleteTextures(1, &m_folderIcon);
    if (m_fileIcon)   glDeleteTextures(1, &m_fileIcon);
    for (auto& [path, tex] : m_thumbnailCache)
        glDeleteTextures(1, &tex);
}

GLuint AssetBrowserPanel::LoadTextureFromFile(const std::string& _path)
{
    int l_w, l_h, l_channels;
    unsigned char* l_data = stbi_load(_path.c_str(), &l_w, &l_h, &l_channels, 4);
    if (!l_data) return 0;

    GLuint l_tex;
    glGenTextures(1, &l_tex);
    glBindTexture(GL_TEXTURE_2D, l_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, l_w, l_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, l_data);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(l_data);
    return l_tex;
}

GLuint AssetBrowserPanel::GetOrLoadThumbnail(const std::string& _path)
{
    auto l_it = m_thumbnailCache.find(_path);
    if (l_it != m_thumbnailCache.end())
        return l_it->second;

    GLuint l_tex = LoadTextureFromFile(_path);
    m_thumbnailCache[_path] = l_tex;
    return l_tex;
}

void AssetBrowserPanel::LoadIcons(const std::string& _folderIconPath, const std::string& _fileIconPath)
{
    if (m_folderIcon) glDeleteTextures(1, &m_folderIcon);
    if (m_fileIcon)   glDeleteTextures(1, &m_fileIcon);
    m_folderIcon = LoadTextureFromFile(_folderIconPath);
    m_fileIcon   = LoadTextureFromFile(_fileIconPath);
}

static bool IsImageExt(const std::string& _ext)
{
    return _ext == ".png" || _ext == ".jpg" || _ext == ".jpeg" || _ext == ".bmp" || _ext == ".tga";
}

static bool IsModelExt(const std::string& _ext)
{
    return _ext == ".obj";
}

void AssetBrowserPanel::DrawDirectoryTree(const std::filesystem::path& _path)
{
    std::vector<std::filesystem::directory_entry> l_dirs;
    std::vector<std::filesystem::directory_entry> l_files;

    try
    {
        for (auto& l_entry : std::filesystem::directory_iterator(_path))
        {
            if (l_entry.is_directory())
                l_dirs.push_back(l_entry);
            else
                l_files.push_back(l_entry);
        }
    }
    catch (const std::filesystem::filesystem_error&)
    {
        return;
    }

    auto l_sortByName = [](const std::filesystem::directory_entry& a,
                           const std::filesystem::directory_entry& b) {
        return a.path().filename().string() < b.path().filename().string();
    };
    std::sort(l_dirs.begin(), l_dirs.end(), l_sortByName);
    std::sort(l_files.begin(), l_files.end(), l_sortByName);

    const float l_iconSize = 16.0f;

    for (auto& l_dir : l_dirs)
    {
        std::string l_name = l_dir.path().filename().string();
        if (!l_name.empty() && l_name[0] == '.') continue;

        // Folder icon
        if (m_folderIcon)
        {
            ImGui::Image((ImTextureID)(intptr_t)m_folderIcon, ImVec2(l_iconSize, l_iconSize));
            ImGui::SameLine();
        }

        if (ImGui::TreeNode(l_name.c_str()))
        {
            DrawDirectoryTree(l_dir.path());
            ImGui::TreePop();
        }
    }

    for (auto& l_file : l_files)
    {
        std::string l_name = l_file.path().filename().string();
        if (!l_name.empty() && l_name[0] == '.') continue;

        std::string l_ext = l_file.path().extension().string();
        std::string l_fullPath = l_file.path().string();

        // File icon
        if (m_fileIcon)
        {
            ImGui::Image((ImTextureID)(intptr_t)m_fileIcon, ImVec2(l_iconSize, l_iconSize));
            ImGui::SameLine();
        }

        // Label with type prefix
        std::string l_label;
        if (l_ext == ".lua")
            l_label = "[Lua] " + l_name;
        else if (l_ext == ".json")
            l_label = "[JSON] " + l_name;
        else if (IsImageExt(l_ext))
            l_label = "[Img] " + l_name;
        else if (IsModelExt(l_ext))
            l_label = "[Model] " + l_name;
        else if (l_ext == ".wav" || l_ext == ".mp3" || l_ext == ".ogg")
            l_label = "[Audio] " + l_name;
        else if (l_ext == ".vs" || l_ext == ".fs" || l_ext == ".vert" || l_ext == ".frag")
            l_label = "[Shader] " + l_name;
        else
            l_label = l_name;

        ImGui::PushID(l_fullPath.c_str());
        if (ImGui::Selectable(l_label.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
        {
            if (ImGui::IsMouseDoubleClicked(0))
            {
                if (l_ext == ".lua")
                {
                    m_editor.SetState(Editor::State::TextEdit);
                }
            }
        }

        // Drag-drop source for textures and models
        if (IsImageExt(l_ext))
        {
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                ImGui::SetDragDropPayload(k_DragDropTexture, l_fullPath.c_str(), l_fullPath.size() + 1);

                // Show thumbnail preview while dragging
                GLuint l_thumb = GetOrLoadThumbnail(l_fullPath);
                if (l_thumb)
                    ImGui::Image((ImTextureID)(intptr_t)l_thumb, ImVec2(48, 48));
                else
                    ImGui::Text("%s", l_name.c_str());

                ImGui::EndDragDropSource();
            }
        }
        else if (IsModelExt(l_ext))
        {
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                ImGui::SetDragDropPayload(k_DragDropModel, l_fullPath.c_str(), l_fullPath.size() + 1);
                ImGui::Text("[Model] %s", l_name.c_str());
                ImGui::EndDragDropSource();
            }
        }

        // Tooltip: thumbnail for images, path for others
        if (ImGui::IsItemHovered())
        {
            if (IsImageExt(l_ext))
            {
                GLuint l_thumb = GetOrLoadThumbnail(l_fullPath);
                if (l_thumb)
                {
                    ImGui::BeginTooltip();
                    ImGui::Image((ImTextureID)(intptr_t)l_thumb, ImVec2(128, 128));
                    ImGui::Text("%s", l_fullPath.c_str());
                    ImGui::EndTooltip();
                }
                else
                {
                    ImGui::SetTooltip("%s", l_fullPath.c_str());
                }
            }
            else
            {
                ImGui::SetTooltip("%s", l_fullPath.c_str());
            }
        }

        ImGui::PopID();
    }
}

void AssetBrowserPanel::Draw()
{
    if (m_rootPath.empty())
    {
        ImGui::Text("No project loaded");
        return;
    }

    ImGui::Text("Project: %s", std::filesystem::path(m_rootPath).filename().string().c_str());
    ImGui::Separator();

    if (ImGui::BeginChild("AssetTree", ImVec2(0, 0), ImGuiChildFlags_Borders))
    {
        DrawDirectoryTree(std::filesystem::path(m_rootPath));
    }
    ImGui::EndChild();
}
