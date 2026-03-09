#include "RanakEngine/Asset/LuaFile.h"
#include "RanakEngine/Core/LuaContext.h"

#include <sstream>
#include <memory>

namespace RanakEngine::Asset
{
    LuaFile::LuaFile(std::string _filePath)
    : AssetFile(_filePath, AssetType::LUA)
    , m_toBeReloaded(false)
    , m_name()
    {
        int l_nameStart = _filePath.find_last_of('/');
        int l_dotPos = _filePath.find_last_of('.');
        m_name = _filePath.substr(l_nameStart + 1, l_dotPos - l_nameStart - 1);

        printf("LuaFile %s created!\n", m_name.c_str());
    }

    LuaFile::~LuaFile()
    {

    }

    std::string LuaFile::GetName()
    {
        return m_name;
    }

    std::string LuaFile::GetCode()
    {
        std::string l_toReturn = std::string(m_contents.data());
        return l_toReturn;
    }
}