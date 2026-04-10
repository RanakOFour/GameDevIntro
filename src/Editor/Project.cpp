#include "Editor/Project.h"

#include <cstdio>

Project Project::Create(const std::string& _rootPath)
{
    static const std::string l_subdirs[] = {
        "Categories", "Rules", "Scenes",
        "Textures",   "Audio", "Models"
    };

    std::filesystem::path l_root(_rootPath);
    for (const std::string& l_sub : l_subdirs)
    {
        std::filesystem::path l_dir = l_root / l_sub;
        if (!std::filesystem::exists(l_dir))
        {
            std::filesystem::create_directories(l_dir);
            printf("Project: Created directory: %s\n", l_dir.string().c_str());
        }
    }

    return Project(_rootPath);
}

bool Project::IsValid(const std::string& _rootPath)
{
    static const std::string l_expected[] = {
        "Categories", "Rules", "Scenes"
    };

    std::filesystem::path l_root(_rootPath);
    for (const std::string& l_sub : l_expected)
    {
        if (std::filesystem::exists(l_root / l_sub))
            return true;
    }
    return false;
}
