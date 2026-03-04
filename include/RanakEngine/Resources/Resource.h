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

        inline std::string GetPath() { return m_path; };
        inline std::string GetContents() { return std::string(m_contents.data()); };
        inline ResourceType GetResourceType() { return m_type; };
    };
}

#endif