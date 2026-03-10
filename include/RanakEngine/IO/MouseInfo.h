#ifndef MOUSEINFO_H
#define MOUSEINFO_H

#include "RanakEngine/Math.h"

namespace RanakEngine::IO
{
    /**
     * @struct MouseInfo
     * @brief Contains mouse input state and position information.
     */
    struct MouseInfo
    {
        Vector2 deltaPosition; ///< Mouse movement deltas since last frame
        
        float deltaScroll;

        Vector2 position; ///< Current mouse position in screen coordinates
        bool RMBDown; ///< Whether the right mouse button is currently pressed
        bool LMBDown; ///< Whether the left mouse button is currently pressed
    };
}

#endif