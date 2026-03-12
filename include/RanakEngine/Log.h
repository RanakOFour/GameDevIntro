#ifndef RANAKLOG_H
#define RANAKLOG_H

#include "RanakEngine/Log/LogManager.h"
#include "sol/sol.hpp"

namespace RanakEngine::Log
{
    namespace
    {
        std::shared_ptr<Log::Manager> LogManager;
    };

    static void Message(std::string _message)
    {
        LogManager->LogMessage(Log::MessageContent::NORMAL, _message);
    }

    static void Debug(std::string _message)
    {
        LogManager->LogMessage(Log::MessageContent::DEBUG, _message);
    }

    static void Warning(std::string _message)
    {
        LogManager->LogMessage(Log::MessageContent::WARNING, _message);
    }

    static void Error(std::string _message)
    {
        LogManager->LogMessage(Log::MessageContent::ERROR, _message);
    }

    std::string Table(sol::table);

    void DefineLuaLib();

    void Init();
    void Stop();
}

#endif