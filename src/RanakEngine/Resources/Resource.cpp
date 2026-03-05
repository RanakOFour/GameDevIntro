#include "RanakEngine/Resources/Resource.h"

namespace RanakEngine::Resources
{
    Resource::Resource(std::string _filePath, ResourceType _type) 
    : m_contents()
    , m_type(_type)
    , m_path(_filePath)
    {
    }

    Resource::~Resource()
    {
        
    }

    std::string Resource::GetPath()
    {
        return m_path;
    }

    std::string Resource::GetContents()
    {
        return std::string(m_contents.data());
    }

    ResourceType Resource::GetResourceType()
    {
        return m_type;
    }
}