#ifndef IOMANAGER_H
#define IOMANAGER_H

#include <memory>

#include "RanakEngine/Math/Vector2.h"
#include "RanakEngine/IO/KBInfo.h"
#include "RanakEngine/IO/MouseInfo.h"

namespace RanakEngine::Core
{
    class Manager;
    class Scene;
}

namespace RanakEngine::IO
{
    class Window;
    class Audio;
    class Manager
    {
        friend Core::Manager;
        private:
        static inline std::weak_ptr<IO::Manager> m_self;
        std::weak_ptr<Core::Manager> m_core;
        std::shared_ptr<Window> m_window;
        std::shared_ptr<Audio> m_audio;
        KBInfo m_kbInfo;
        MouseInfo m_mouseInfo;

        void SetCore(std::weak_ptr<Core::Manager> _core);

        Manager(Vector2 _screenSize);
        public:
        ~Manager();

        static std::shared_ptr<IO::Manager> Init(Vector2 _screenSize);
        static std::weak_ptr<IO::Manager> Instance();

        std::string OpenFileDialog();
        std::string SaveFileDialog();

        void UpdateInputs();

        bool GetKeyDown(char _key);
        bool WindowFocused();

        void SetScreenSize(Vector2 _size);
        Vector2 GetScreenSize();

        void SetClearColour(Vector4 _colour);
        Vector4 GetClearColour();

        void Draw(std::shared_ptr<Core::Scene> _sceneToDraw);

        std::weak_ptr<Window> GetWindow();
        std::weak_ptr<Audio> GetAudio();
        MouseInfo* GetMouseInfo();
        KBInfo GetKBInfo();
    };
}

#endif