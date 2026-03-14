#include "RanakEngine/Core/CoreManager.h"
#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Core/Scene.h"
#include "RanakEngine/Core/Camera.h"
#include "RanakEngine/IO/IOManager.h"
#include "RanakEngine/Log.h"

#include "SDL3/SDL.h"

namespace RanakEngine::Core
{
    Core::Manager::Manager(bool _debug)
    : m_running(false)
    , m_debug(_debug)
    , m_deltaTime(0.0f)
    , m_targetFPS(60.0f)
    {
        m_ioManager = IO::Manager::Instance();
        m_luaContext = LuaContext::Init();
        m_currentScene = std::make_shared<Scene>();
        m_mainCamera = std::make_shared<Camera>();
    };

    Core::Manager::~Manager()
    {
        m_luaContext.reset();
    }

    std::shared_ptr<Core::Manager> Core::Manager::Init(bool _debug)
    {
        if (m_self.lock() == nullptr)
        {
            std::shared_ptr<Core::Manager> l_toReturn;
            Core::Manager *l_manager = new Core::Manager(_debug);
            l_toReturn.reset(l_manager);

            l_toReturn->m_ioManager.lock()->SetCore(l_toReturn);

            l_toReturn->m_self = l_toReturn;

            return l_toReturn;
        }

        return m_self.lock();
    };

    std::weak_ptr<Core::Manager> Core::Manager::Instance()
    {
        return m_self;
    }

    void Core::Manager::Start()
    {
        m_running = true;

        float l_oneOverTargetFPS = 1.0f / m_targetFPS;
        auto l_IOManager = m_ioManager.lock();

        Uint64 l_startCounters = SDL_GetPerformanceCounter();
        Uint64 l_currentCounters = 0.0f;

        m_currentScene->Init();

        while (m_running)
        {
            l_startCounters = l_currentCounters;
            l_currentCounters = SDL_GetPerformanceCounter();

            m_deltaTime = ((float)(l_currentCounters - l_startCounters) / (float)SDL_GetPerformanceFrequency());
            Log::Message("DeltaTime: " + std::to_string(m_deltaTime) + "s");

            // Update IO
            l_IOManager->UpdateInputs();

            // if (l_IOManager->WindowFocused())
            // {
            //     // Do systems
            //     m_currentScene->Update(m_deltaTime);

            //     // m_Physics.lock()->Update(m_deltaTime);
            // }

            m_currentScene->Update(m_deltaTime);

            l_IOManager->Draw(m_currentScene);

            float l_waitTime = l_oneOverTargetFPS - m_deltaTime;

            if (l_waitTime > 0.0f)
            {
                SDL_Delay((Uint32)(l_waitTime * 1000.0f));
            }
        }
    }

    void Core::Manager::Stop()
    {
        m_running = false;
    };

    void Core::Manager::SetScene(std::shared_ptr<Scene> _scene)
    {
        m_currentScene = _scene;
    }

    std::weak_ptr<Scene> Core::Manager::GetScene()
    {
        return m_currentScene;
    }

    float Core::Manager::DeltaTime()
    {
        return m_deltaTime;
    }

    bool Core::Manager::IsDebug()
    {
        return m_debug;
    }

    std::weak_ptr<Camera> Core::Manager::GetCamera()
    {
        return m_mainCamera;
    }
}