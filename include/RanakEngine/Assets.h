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
    std::shared_ptr<Asset::Manager> Init();
    void Stop();
}

#endif