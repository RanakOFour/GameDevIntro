#ifndef KBINFO_H
#define KBINFO_H

namespace RanakEngine::IO
{
    /**
     * @struct KBInfo
     * @brief Contains keyboard input state.
     * 
     * Maintains an array of boolean flags for all 322 SDL keyboard key codes,
     * allowing quick lookup of key state.
     */
    struct KBInfo
    {
        int kbSize; ///< Size of the keyboard input map (always 322 for SDL keys)

        // 322 SDLk_* keys
        bool inputMap[322]; ///< Array tracking the state of each keyboard key
    };
}

#endif