#include "RanakEngine/Asset/AssetFile.h"

namespace RanakEngine::Asset
{
    AssetFile::AssetFile(std::string _filePath, AssetType _type) 
    : m_contents()
    , m_type(_type)
    , m_path(_filePath)
    {
    }

    AssetFile::~AssetFile()
    {
        
    }

    std::string AssetFile::GetPath()
    {
        return m_path;
    }

    std::string AssetFile::GetContents()
    {
        return std::string(m_contents.data());
    }

    AssetType AssetFile::GetAssetType()
    {
        return m_type;
    }
}