#ifndef COREMANAGER_H
#define COREMANAGER_H

#include <memory>

namespace RanakEngine
{
    namespace IO
    {
        class Manager;
    };

    namespace Physics
    {
        class Manager;
    }

    namespace Asset
    {
        class LuaFile;
    }
}

namespace RanakEngine::Core
{
    class LuaContext;
    class Category;
    class CategoryFactory;
    class Scene;
    class Camera;
    class Manager
    {
    private:
        static inline std::weak_ptr<Core::Manager> m_self;
        std::weak_ptr<IO::Manager> m_ioManager;
        std::weak_ptr<Physics::Manager> m_physicsManager;

        std::shared_ptr<LuaContext> m_luaContext;
        std::shared_ptr<Camera> m_mainCamera;

        std::shared_ptr<Scene> m_currentScene;
        bool m_running;
        bool m_debug;
        float m_deltaTime;
        float m_targetFPS;

        Manager(bool _debug);

    public:
        ~Manager();

        static std::shared_ptr<Core::Manager> Init(bool _debug);
        static std::weak_ptr<Core::Manager> Instance();

        void Start();
        void Stop();

        void SetScene(std::shared_ptr<Scene> _scene);
        std::weak_ptr<Scene> GetScene();

        float DeltaTime();

        bool IsDebug();

        std::weak_ptr<Camera> GetCamera();
    };
}

#endif