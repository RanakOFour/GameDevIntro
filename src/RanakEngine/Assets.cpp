#include "RanakEngine/Assets.h"

#include "RanakEngine/Core/LuaContext.h"
#include "sol/sol.hpp"

namespace RanakEngine::Asset
{
    namespace
    {
        static std::shared_ptr<Asset::Manager> AssetManager;
        static sol::table AssetTable;
    }

    void DefineLuaTypes()
    {
        auto l_context = Core::LuaContext::Instance().lock();

        AssetTable = l_context->CreateTable();

        l_context->SetGlobal("Asset", AssetTable);
    }

    std::shared_ptr<Asset::Manager> Init()
    {
        AssetManager = Asset::Manager::Init();
        //DefineLuaTypes();
        return AssetManager;
    }

    void Stop()
    {
        AssetManager.reset();
        AssetTable.clear();
    }
}