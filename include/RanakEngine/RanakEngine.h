#ifndef RANAKENGINE_H
#define RANAKENGINE_H

#include <memory>

#include "RanakEngine/Math.h"
#include "RanakEngine/Log.h"
#include "RanakEngine/Assets.h"
#include "RanakEngine/Core.h"
#include "RanakEngine/IO.h"
// #include "RanakEngine/Physics.h"


namespace RanakEngine
{
    struct EngineContents
    {
        std::shared_ptr<Asset::Manager> resources;
        std::shared_ptr<Core::Manager> core;
        std::shared_ptr<IO::Manager> io;
        // std::shared_ptr<Physics::Manager> physics;
    };

    EngineContents Initialise(bool _debug, Vector2 _screenSize)
    {
        EngineContents l_toReturn;

        Log::Init();

        l_toReturn.resources = Asset::Init();
        l_toReturn.io = IO::Init(_screenSize);
        l_toReturn.core = Core::Init(_debug);
        // l_toReturn.physics = Physics::Init();



        return l_toReturn;
    };

    void Shutdown(EngineContents& _contents)
    {
        // Physics::Stop();
        RanakEngine::IO::Stop();
        RanakEngine::Log::Stop();
        
        Asset::Stop();
        IO::Stop();
        Core::Stop();
    }
}

namespace RE = RanakEngine;

#endif