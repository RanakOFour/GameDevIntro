#ifndef RANAKRESOURCES_H
#define RANAKRESOURCES_H

#include "RanakEngine/Asset/LuaFile.h"
#include "RanakEngine/Asset/Audio.h"
#include "RanakEngine/Asset/Model.h"
#include "RanakEngine/Asset/Shader.h"
#include "RanakEngine/Asset/Texture.h"
#include "RanakEngine/Asset/AssetManager.h"

namespace RanakEngine::Asset
{
    namespace
    {
        static std::shared_ptr<Asset::Manager> AssetManager;
    }

    template<typename T>
    static std::weak_ptr<T> Load(std::string _path)
    {
        return AssetManager->Load<T>(_path);
    };

    void DefineLuaLib();
    std::shared_ptr<Asset::Manager> Init();
    void Stop();
}

#endif