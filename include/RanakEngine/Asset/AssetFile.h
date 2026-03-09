#ifndef ASSET_H
#define ASSET_H

#include <string>
#include <vector>

namespace RanakEngine::Asset
{
    enum AssetType
    {
        TEXTURE,
        MODEL,
        SHADER,
        AUDIO,
        LUA
    };

    class AssetFile
    {
        protected:
        std::string m_path;
        std::vector<char> m_contents;
        AssetType m_type;

        public:
        AssetFile(std::string _path, AssetType _type);
        ~AssetFile();

        std::string GetPath();
        std::string GetContents();
        AssetType GetAssetType();
    };
}

#endif