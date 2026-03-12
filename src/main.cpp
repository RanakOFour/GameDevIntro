#include "RanakEngine/RanakEngine.h"

int main()
{
    auto l_contents = RE::Initialise(true, Vector2(1280, 720));

    RE::Log::Message("Engine Initialised\n");

    RE::Shutdown(l_contents);
    return 0;
}