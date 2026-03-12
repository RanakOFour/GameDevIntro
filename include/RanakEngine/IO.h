#ifndef RANAKIO_H
#define RANAKIO_H

#include "RanakEngine/IO/MouseInfo.h"
#include "RanakEngine/IO/KBInfo.h"
#include "RanakEngine/IO/Audio.h"
#include "RanakEngine/IO/Window.h"
#include "RanakEngine/IO/IOManager.h"

namespace RanakEngine::IO
{
    void DefineLuaLib();
    std::shared_ptr<IO::Manager> Init(Vector2 _screenSize);
    void Stop();
}

#endif