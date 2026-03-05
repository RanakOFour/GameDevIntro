#ifndef RANAKENGINE_H
#define RANAKENGINE_H

#include <memory>

#include "RanakEngine/LuaContext.h"
#include "RanakEngine/Math.h"
#include "RanakEngine/Log.h"
#include "RanakEngine/Resources.h"
#include "RanakEngine/Core.h"
#include "RanakEngine/IO.h"
// #include "RanakEngine/Phsyics.h"

namespace RanakEngine
{
    struct EngineContents
    {
        std::shared_ptr<Log::Manager> logger;
        std::shared_ptr<Resources::Manager> resources;
        std::shared_ptr<Core::Manager> core;
        std::shared_ptr<IO::Manager> io;
        // std::shared_ptr<Physics::Manager> physics;
    };

    EngineContents Initialise(bool _debug, Vector2 _screenSize)
    {
        EngineContents l_toReturn;
        l_toReturn.logger = Log::Manager::Init();
        l_toReturn.resources = Resources::Manager::Init();
        l_toReturn.core = Core::Manager::Init(_debug);
        l_toReturn.io = IO::Manager::Init(_screenSize);
        // l_toReturn.physics = Physics::Manager::Init();

        return l_toReturn;
    };

    void Shutdown(EngineContents& _contents)
    {
        // _contents.physics->Stop()
        _contents.io->Stop();
        _contents.core->Stop();
        _contents.logger->Stop();
    }
}

namespace RE = RanakEngine;

#endif