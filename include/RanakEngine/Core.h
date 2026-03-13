#ifndef RANAKCORE_H
#define RANAKCORE_H

#include "RanakEngine/Core/Camera.h"
#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Core/CoreManager.h"
#include "RanakEngine/Core/CategoryFactory.h"
#include "RanakEngine/Core/Category.h"
#include "RanakEngine/Core/Scene.h"
#include "RanakEngine/Core/Rule.h"

#include "sol/sol.hpp"

namespace RanakEngine::Core
{
    void DefineLuaLib();
    std::shared_ptr<Core::Manager> Init(bool _isDebug);
    void Stop();
}

#endif