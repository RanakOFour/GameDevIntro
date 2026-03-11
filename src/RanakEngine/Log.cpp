#include "RanakEngine/Log.h"
#include "RanakEngine/Core/LuaContext.h"
#include "sol/sol.hpp"

namespace RanakEngine::Log
{
    namespace
    {
        static sol::table LogTable;
        
        void DefineLuaTypes()
        {
            auto l_context = Core::LuaContext::Instance().lock();

            LogTable = l_context->CreateTable();

            LogTable.set_function("Message", [](const std::string& _message) { LogManager->LogMessage(Log::MessageContent::NORMAL, _message); });
            LogTable.set_function("Debug", [](const std::string& _message) { LogManager->LogMessage(Log::MessageContent::DEBUG, _message); });
            LogTable.set_function("Warning", [](const std::string& _message) { LogManager->LogMessage(Log::MessageContent::WARNING, _message); });
            LogTable.set_function("Error", [](const std::string& _message) { LogManager->LogMessage(Log::MessageContent::ERROR, _message); });

            l_context->SetGlobal("Log", LogTable);
        }
    };

    void Init()
    {
        LogManager = Log::Manager::Init();
        DefineLuaTypes();
    }

    void Stop()
    {
        LogManager->Stop();
        LogManager.reset();
        LogTable.clear();
    }   
}