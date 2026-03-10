#ifndef RANAKCORE_H
#define RANAKCORE_H

#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Core/CoreManager.h"
#include "RanakEngine/Core/CategoryFactory.h"
#include "RanakEngine/Core/Category.h"
#include "RanakEngine/Core/Scene.h"
#include "RanakEngine/Core/Rule.h"

#include "sol/sol.hpp"

namespace RanakEngine::Core
{
    // Unnamed namespace for 'internal' Lua bindings
    namespace
    {
        static sol::table CoreTable;
        std::shared_ptr<Core::Manager> CoreManager;

        void DefineLuaTypes()
        {
            auto l_context = LuaContext::Instance().lock();

            CoreTable = l_context->CreateTable();

            l_context->SetGlobal("Core", CoreTable);
        }
    }

    std::shared_ptr<Core::Manager> Init(bool _isDebug)
    {
        CoreManager = Core::Manager::Init(_isDebug);
        DefineLuaTypes();
        return CoreManager;
    }

    void Stop()
    {
        CoreManager.reset();
        CoreTable.clear();
    }
}

#endif