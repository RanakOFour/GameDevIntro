#ifndef IOMANAGER_H
#define IOMANAGER_H

#include <memory>

#include "RanakEngine/IO/KBInfo.h"
#include "RanakEngine/IO/MouseInfo.h"

namespace RanakEngine::Core
{
    class Manager;
}

namespace RanakEngine::IO
{
    class Window;
    class Audio;
    class Manager
    {
        private:
        static inline std::weak_ptr<IO::Manager> m_self;
        std::weak_ptr<Core::Manager> m_core;
        std::shared_ptr<Window> m_window;
        std::shared_ptr<Audio> m_audio;
        KBInfo m_kbInfo;
        MouseInfo m_mouseInfo;

        Manager();
        public:
        ~Manager();

        static std::shared_ptr<IO::Manager> Init();
        static std::weak_ptr<IO::Manager> Instance();

        std::string OpenFileDialog();
        std::string SaveFileDialog();

        void UpdateInputs();
        std::weak_ptr<Window> GetWindow();
        std::weak_ptr<Audio> GetAudio();

        bool GetKeyDown(char _key);
        std::weak_ptr<MouseInfo> GetMouseInfo();
    };
}

#endif