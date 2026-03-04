#ifndef LUAFILE_H
#define LUAFILE_H

#include <string>

#include "Resources/Resource.h"

namespace RanakEngine::Resources
{
    class LuaFile : public Resource
    {
        public:
        enum FileContents
        {
            Category,
            Rule    
        };

        private:
        bool m_toBeReloaded;
        std::string m_name;
        FileContents m_contents;

        public:
        LuaFile(std::string _filePath);
        ~LuaFile();

        void Reload();
        void Run();

        std::string GetName();
        std::string GetCode();
        FileContents GetContents();
    };
}

#endif