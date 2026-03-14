#include "RanakEngine/IO/Window.h"
#include "RanakEngine/Log.h"

bool InitGL()
{
    std::string l_logMessage;
    // GLEW has a problem with loading core OpenGL
    // See here: https://www.opengl.org/wiki/OpenGL_Loading_Library
    // The temporary workaround is to enable its 'experimental' features
    glewExperimental = GL_TRUE;

    GLenum err = glewInit();
    // After updating to SDL3, glewInit() returns "4", even though there is no issue
    if (GLEW_OK != err && err != 4)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        l_logMessage = "GLEW failed to initialise with message: " +
                        std::string((char*)glewGetErrorString(err)) +
                        "\nErr num: " + std::to_string(err);
        RanakEngine::Log::Error(l_logMessage);
        return false;
    }

    l_logMessage = "INFO: Using GLEW " + std::string((char*)glewGetString(GLEW_VERSION))
    + "\nINFO: OpenGL Vendor: " + std::string((char*)glGetString(GL_VENDOR))
    + "\nINFO: OpenGL Renderer: " + std::string((char*)glGetString(GL_RENDERER))
    + "\nINFO: OpenGL Version: " + std::string((char*)glGetString(GL_VERSION))
    + "\nINFO: OpenGL Shading Language Version: " + std::string((char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    RanakEngine::Log::Message(l_logMessage);

    return true;
}

namespace RanakEngine::IO
{
    Window::Window(Vector2 _screenSize, std::string _title)
    : m_clearColour(0.0f, 0.0f, 0.2f, 1.0f)
    , m_screenSize(_screenSize)
    , m_aspectRatio(_screenSize.x / m_screenSize.y)
    {
        if(SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            Log::Error("Cannot initialise SDL");
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        int l_winWidth = _screenSize.x;
        int l_winHeight = _screenSize.y;

        m_sdlWindow = std::shared_ptr<SDL_Window>(
            SDL_CreateWindow(_title.c_str(),
                            l_winWidth, l_winHeight,
                            SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL),
            SDL_DestroyWindow);

        m_sdlglContext = SDL_GL_CreateContext(m_sdlWindow.get());

        if(!InitGL())
        {
            Log::Error("Could not initialise GL");
        }
    }

    Window::~Window()
    {
        if(m_sdlglContext)
        {
            SDL_GL_DestroyContext(m_sdlglContext);
        }

        if(m_sdlWindow.get())
        {
            SDL_DestroyWindow(m_sdlWindow.get());
        }
    }

    void Window::Swap()
    {
        SDL_GL_SwapWindow(m_sdlWindow.get());
    }

    bool Window::IsMouseInside()
    {
        return SDL_GetWindowRelativeMouseMode(m_sdlWindow.get());
    }

    void Window::SetRelativeWindow(bool _value)
    {
        SDL_Window* l_window = m_sdlWindow.get();
        SDL_SetWindowRelativeMouseMode(l_window, !SDL_GetWindowRelativeMouseMode(l_window));
    }

    bool Window::GetRelativeWindow()
    {
        return SDL_GetWindowRelativeMouseMode(m_sdlWindow.get());
    }

    void Window::SetScreenSize(Vector2 _size)
    {
        m_screenSize = _size;
        m_aspectRatio = m_screenSize.x / m_screenSize.y;
    }

    Vector2 Window::GetScreenSize()
    {
        return m_screenSize;
    }

    void Window::SetClearColour(Vector4 _colour)
    {
        m_clearColour = _colour;
    }

    Vector4 Window::GetClearColour()
    {
        return m_clearColour;
    }
}