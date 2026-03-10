#ifndef RANAKLOG_H
#define RANAKLOG_H

#include "RanakEngine/Log/LogManager.h"

#include "RanakEngine/Core/LuaContext.h"
#include "sol/sol.hpp"

namespace RanakEngine::Log
{
    // Unnamed namespace for 'internal' Lua bindings
    namespace
    {
        static sol::table LogTable;
        std::shared_ptr<Log::Manager> LogManager;

        void DefineLuaTypes()
        {
            auto l_context = Core::LuaContext::Instance().lock();

            LogTable = l_context->CreateTable();

            LogTable.set_function("Message", [](const std::string& _message) { LogManager->LogMessage(Log::Message::NORMAL, _message); });
            LogTable.set_function("Debug", [](const std::string& _message) { LogManager->LogMessage(Log::Message::DEBUG, _message); });
            LogTable.set_function("Warning", [](const std::string& _message) { LogManager->LogMessage(Log::Message::WARNING, _message); });
            LogTable.set_function("Error", [](const std::string& _message) { LogManager->LogMessage(Log::Message::ERROR, _message); });

            l_context->SetGlobal("Log", LogTable);
        }
    };

    std::shared_ptr<Log::Manager> Init()
    {
        LogManager = Log::Manager::Init();
        DefineLuaTypes();
        return LogManager;
    }

    void Stop()
    {
        LogManager->Stop();
        LogManager.reset();
        LogTable.clear();
    }
}

#endif