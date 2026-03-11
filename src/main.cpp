#include "RanakEngine/RanakEngine.h"

int main()
{
    auto l_contents = RE::Initialise(true, Vector2(1280, 720));

    RE::Shutdown(l_contents);
    return 0;
}