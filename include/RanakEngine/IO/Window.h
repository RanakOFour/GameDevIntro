#ifndef WINDOW_H
#define WINDOW_H

#include <memory>

#include "RanakEngine/Math.h"

#include "GL/glew.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_audio.h"

namespace RanakEngine::IO
{
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
        Window(Vector2 _screenSize, std::string _title = "RanakEngine");
        ~Window();

        void Swap();

        bool IsMouseInside();
        void SetRelativeWindow(bool _value);
        bool GetRelativeWindow();

        void SetScreenSize(Vector2 _size);
        Vector2 GetScreenSize();

        void SetClearColour(Vector4 _colour);
        Vector4 GetClearColour();

        SDL_Window* GetSDLWindow() const { return m_sdlWindow.get(); }
        SDL_GLContext GetGLContext() const { return m_sdlglContext; }
        SDL_AudioStream* GetAudioStream() const { return m_audioStream.get(); }
    };
}

#endif