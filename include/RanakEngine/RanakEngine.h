#pragma once
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

    EngineContents Initialise(bool _debug, Vector2 _screenSize);
    void Shutdown(EngineContents &_contents);
}

namespace RE = RanakEngine;

#endif