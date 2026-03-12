#include "RanakEngine/Asset/LuaFile.h"
#include "RanakEngine/Core/LuaContext.h"

#include <fstream> 
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

        // Open file
        std::ifstream l_stream;
        l_stream.open(_filePath);

        // Get size of file
        l_stream.seekg(0,std::ios::end);
        std::streampos l_fileLength = l_stream.tellg();
        l_stream.seekg(0,std::ios::beg);

        // Read file into content vector
        m_contents.resize(l_fileLength);
        l_stream.read(&m_contents[0], l_fileLength);

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