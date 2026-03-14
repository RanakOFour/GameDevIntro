#include "RanakEngine/Core.h"

#include "RanakEngine/Core/LuaContext.h"
#include "sol/sol.hpp"

namespace RanakEngine::Core
{
    namespace
    {
        static std::shared_ptr<Core::Manager> CoreManager;
        static sol::table CoreTable;
    }

    void DefineLuaLib()
    {
        auto l_context = LuaContext::Instance().lock();

        CoreTable = l_context->CreateTable();

        CoreTable.set_function("DeltaTime", [](){return CoreManager->DeltaTime();});
        
        std::weak_ptr<Camera> l_camera = CoreManager->GetCamera();
        CoreTable.set("Camera", l_camera.lock());

        l_context->SetGlobal("Core", CoreTable);
    }

    std::shared_ptr<Core::Manager> Init(bool _isDebug)
    {
        CoreManager = Core::Manager::Init(_isDebug);
        return CoreManager;
    }

    void Stop()
    {
        CoreTable.clear();
        CoreManager.reset();
    }
}