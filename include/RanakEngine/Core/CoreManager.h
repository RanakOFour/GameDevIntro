#ifndef COREMANAGER_H
#define COREMANAGER_H

#include <memory>

namespace RanakEngine::IO
{
    class Manager;
}


namespace RanakEngine::Physics
{
    class Manager;
}

class LuaContext;
class Scene;

namespace RanakEngine::Core
{
    class Manager
    {
        private:
        static inline std::weak_ptr<Core::Manager> m_self;
        std::weak_ptr<IO::Manager> m_ioManager;
        std::weak_ptr<Physics::Manager> m_physicsManager;
        std::weak_ptr<LuaContext> m_luaContext;

        std::shared_ptr<Scene> m_currentScene;
        bool m_running;
        bool m_debug;
        bool m_deltaTime;
        bool m_targetFPS;

        void Update();
        Manager(bool _debug);

        public:
        ~Manager();

        static std::shared_ptr<Core::Manager> Init();
        static std::weak_ptr<Core::Manager> Instance();

        void Start();
        void Stop();

        void SetScene(std::shared_ptr<Scene> _scene);
        std::weak_ptr<Scene> GetScene();

        bool IsDebug();
    };
}

#endif