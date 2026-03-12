#include "RanakEngine/IO.h"

#include "RanakEngine/Core/LuaContext.h"
#include "sol/sol.hpp"

namespace RanakEngine::IO
{
    namespace
    {
        static sol::table IOTable;
        static std::shared_ptr<IO::Manager> IOManager;
    };

    void DefineLuaLib()
    {
        auto l_context = Core::LuaContext::Instance().lock();

        IOTable = l_context->CreateTable();

        IOTable.new_usertype<MouseInfo>("MouseInfo",
                                        "position", &MouseInfo::position,
                                        "deltaPosition", &MouseInfo::deltaPosition,
                                        "scrollDelta", &MouseInfo::deltaScroll,
                                        "RMBDown", &MouseInfo::RMBDown,
                                        "LMBDown", &MouseInfo::LMBDown);

        IOTable.set("MouseInfo", IOManager->GetMouseInfo());

        IOTable.set_function("GetKeyDown", [](char _key)
                             { return IOManager->GetKeyDown(_key); });
        IOTable.set_function("WindowFocused", []()
                             { return IOManager->WindowFocused(); });
        IOTable.set_function("OpenFileDialog", []()
                             { return IOManager->OpenFileDialog(); });
        IOTable.set_function("SaveFileDialog", []()
                             { return IOManager->SaveFileDialog(); });

        IOTable.set_function("SetScreenSize", [](Vector2 _size)
                             { IOManager->SetScreenSize(_size); });
        IOTable.set_function("ScreenSize", []()
                             { return IOManager->GetScreenSize(); });

        IOTable.set_function("SetClearColour", [](Vector4 _colour)
                             { IOManager->SetClearColour(_colour); });
        IOTable.set_function("ClearColour", []()
                             { return IOManager->GetClearColour(); });

        l_context->SetGlobal("IO", IOTable);
    };

    std::shared_ptr<IO::Manager> Init(Vector2 _screenSize)
    {
        IOManager = IO::Manager::Init(_screenSize);
        // DefineLuaTypes();
        return IOManager;
    }

    void Stop()
    {
        IOTable.clear();
        IOManager.reset();
    }
}