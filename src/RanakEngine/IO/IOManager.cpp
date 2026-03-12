#include "RanakEngine/IO/IOManager.h"
#include "RanakEngine/IO/Audio.h"
#include "RanakEngine/IO/Window.h"

#include "RanakEngine/Log/LogManager.h"
#include "RanakEngine/Core/CoreManager.h"
#include "RanakEngine/Core/Scene.h"

namespace RanakEngine::IO
{
    void IO::Manager::SetCore(std::weak_ptr<Core::Manager> _core)
    {
        m_core = _core;
    }

    IO::Manager::Manager(Vector2 _screenSize)
    : m_kbInfo()
    , m_mouseInfo()
    {
        m_window = std::make_shared<Window>(_screenSize);
        m_audio = std::make_shared<Audio>();

        m_kbInfo.kbSize = 322;
    }

    IO::Manager::~Manager()
    {
        
    }

    std::shared_ptr<IO::Manager> IO::Manager::Init(Vector2 _screenSize)
    {
        if(m_self.lock() == nullptr)
        {
            std::shared_ptr<IO::Manager> l_toReturn;
            IO::Manager* l_manager = new IO::Manager(_screenSize);
            l_toReturn.reset(l_manager);
            
            l_toReturn->m_self = l_toReturn;

            glDebugMessageCallback(Log::Manager::GLDebugMessageCallback, 0);

            return l_toReturn;
        }

        return m_self.lock();
    };

    std::weak_ptr<IO::Manager> IO::Manager::Instance()
    {
        return m_self;
    }

    std::string IO::Manager::OpenFileDialog()
    {
        return "";
    }

    std::string IO::Manager::SaveFileDialog()
    {
        return "";
    }

    void IO::Manager::UpdateInputs()
    {
        static bool l_previousESCValue = false;
        bool l_movement = false;
        bool l_scroll = false;
        SDL_Event l_event;

        m_mouseInfo.deltaPosition.x = 0.0f;
        m_mouseInfo.deltaPosition.y = 0.0f;
        m_mouseInfo.deltaScroll = 0.0f;

        while (SDL_PollEvent(&l_event))
        {
            switch (l_event.type)
            {
            case SDL_EVENT_QUIT:
                m_core.lock()->Stop();
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (l_event.button.button == SDL_BUTTON_LEFT)
                {
                    m_mouseInfo.LMBDown = true;
                }
                else
                {
                    m_mouseInfo.RMBDown = true;
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (l_event.button.button == SDL_BUTTON_LEFT)
                {
                    m_mouseInfo.LMBDown = false;
                }
                else
                {
                    m_mouseInfo.RMBDown = false;
                }
                break;

            case SDL_EVENT_MOUSE_WHEEL:
                m_mouseInfo.deltaScroll = -l_event.wheel.y;
                if (m_mouseInfo.deltaScroll != 0.0f)
                {
                    l_scroll = true;
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                m_mouseInfo.deltaPosition.x = l_event.motion.xrel;
                m_mouseInfo.deltaPosition.y = l_event.motion.yrel;

                if (m_mouseInfo.deltaPosition.x != 0.0f && m_mouseInfo.deltaPosition.y != 0.0f)
                {
                    l_movement = true;
                }
                break;

            case SDL_EVENT_KEY_DOWN:
                m_kbInfo.inputMap[l_event.key.scancode] = true;
                break;

            case SDL_EVENT_KEY_UP:
                m_kbInfo.inputMap[l_event.key.scancode] = false;
                break;
            }
        }

        // Take mouse out of window focus on escape
        if (m_kbInfo.inputMap[SDL_SCANCODE_ESCAPE] && !l_previousESCValue)
        {
            m_window->SetRelativeWindow(!m_window->GetRelativeWindow());
        }

        l_previousESCValue = m_kbInfo.inputMap[SDL_SCANCODE_ESCAPE];
    }

    bool IO::Manager::GetKeyDown(char _key)
    {
        int l_sdlCode = SDL_GetScancodeFromKey(_key, NULL);
        return m_kbInfo.inputMap[l_sdlCode];
    }

    bool IO::Manager::WindowFocused()
    {
        return m_window->IsMouseInside();
    }

    void IO::Manager::Draw(std::shared_ptr<Core::Scene> _sceneToDraw)
    {
        Vector4 l_clearColour = m_window->GetClearColour();
        
        glClearColor(l_clearColour.x, l_clearColour.y, l_clearColour.z, l_clearColour.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ImGui_ImplOpenGL3_NewFrame();
        // ImGui_ImplSDL3_NewFrame();
        // ImGui::NewFrame();

        _sceneToDraw->Draw();

        // ImGui::Render();
        // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        m_window->Swap();
    }

    void IO::Manager::SetScreenSize(Vector2 _size)
    {
        m_window->SetScreenSize(_size);
    }

    Vector2 IO::Manager::GetScreenSize()
    {
        return m_window->GetScreenSize();
    }

    void IO::Manager::SetClearColour(Vector4 _colour)
    {
        m_window->SetClearColour(_colour);
    }

    Vector4 IO::Manager::GetClearColour()
    {
        return m_window->GetClearColour();
    }

    std::weak_ptr<Window> IO::Manager::GetWindow()
    {
        return m_window;
    }

    std::weak_ptr<Audio> IO::Manager::GetAudio()
    {
        return m_audio;
    }

    MouseInfo* IO::Manager::GetMouseInfo()
    {
        return &m_mouseInfo;
    }

    KBInfo IO::Manager::GetKBInfo()
    {
        return m_kbInfo;
    }
}