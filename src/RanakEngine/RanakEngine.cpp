#include "RanakEngine/RanakEngine.h"

namespace RanakEngine
{
    EngineContents Initialise(bool _debug, Vector2 _screenSize)
    {
        EngineContents l_toReturn;

        Log::Init();

        l_toReturn.resources = Asset::Init();
        l_toReturn.io = IO::Init(_screenSize);
        l_toReturn.core = Core::Init(_debug);
        // l_toReturn.physics = Physics::Init();

        Math::DefineLuaLib();
        Asset::DefineLuaLib();
        Core::DefineLuaLib();
        IO::DefineLuaLib();
        Log::DefineLuaLib();

        return l_toReturn;
    };

    void Shutdown(EngineContents &_contents)
    {
        // Physics::Stop();
        Log::Message("Stopping core");
        Core::Stop();

        Log::Message("Stopping IO");
        IO::Stop();

        Log::Message("Stopping Asset");
        Asset::Stop();

        Log::Message("Stopping Log");
        Log::Stop();

        Math::Stop();
    }
}