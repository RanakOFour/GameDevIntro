#ifndef LUAFILE_H
#define LUAFILE_H

#include <string>

#include "RanakEngine/Asset/AssetFile.h"

namespace RanakEngine::Asset
{
    class LuaFile : public AssetFile
    {
        public:

        private:
        bool m_toBeReloaded;
        std::string m_name;

        public:
        LuaFile(std::string _filePath);
        ~LuaFile();

        std::string GetName();
        std::string GetCode();
    };
}

#endif