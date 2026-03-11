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

    void DefineLuaTypes()
    {
        auto l_context = LuaContext::Instance().lock();

        CoreTable = l_context->CreateTable();

        l_context->SetGlobal("Core", CoreTable);
    }

    std::shared_ptr<Core::Manager> Init(bool _isDebug)
    {
        CoreManager = Core::Manager::Init(_isDebug);
        //DefineLuaTypes();
        return CoreManager;
    }

    void Stop()
    {
        CoreManager.reset();
        CoreTable.clear();
    }
}