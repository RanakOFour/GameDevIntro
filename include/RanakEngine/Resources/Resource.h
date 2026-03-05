#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>
#include <vector>

namespace RanakEngine::Resources
{
    enum ResourceType
    {
        TEXTURE,
        MODEL,
        SHADER,
        AUDIO,
        LUA
    };

    class Resource
    {
        protected:
        std::string m_path;
        std::vector<char> m_contents;
        ResourceType m_type;

        public:
        Resource(std::string _path, ResourceType _type);
        ~Resource();

        std::string GetPath();
        std::string GetContents();
        ResourceType GetResourceType();
    };
}

#endif