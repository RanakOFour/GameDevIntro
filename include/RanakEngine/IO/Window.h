#ifndef WINDOW_H
#define WINDOW_H

#include <memory>

#include "RanakEngine/Math.h"

#include "GL/glew.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_audio.h"

namespace RanakEngine::IO
{
    class Manager;
    class Window
    {
        private:
        std::shared_ptr<SDL_Window> m_sdlWindow;
        SDL_GLContext m_sdlglContext;
        std::shared_ptr<SDL_AudioStream> m_audioStream;
        Vector4 m_clearColour;
        Vector2 m_screenSize;
        float m_aspectRatio;

        public:
        Window(Vector2 _screenSize);
        ~Window();

        void Swap();

        bool IsMouseInside();
        void SetRelativeWindow(bool _value);
        bool GetRelativeWindow();
        Vector4 GetClearColour();
    };
}

#endif