#include "RanakEngine/Log.h"
#include "RanakEngine/Core/LuaContext.h"
#include "sol/sol.hpp"

namespace RanakEngine::Log
{
    namespace
    {
        static sol::table LogTable;
    };

    std::string Table(sol::table _table)
    {
        printf("Printing table data:\n");
        auto l_tablePairs = _table.pairs();

        // Iterate key-value pairs of the table
        for(auto l_pair : l_tablePairs)
        {
            sol::object l_obj = l_pair.first;
            std::cout << "Entry: ";
            switch(l_obj.get_type())
            {
                case sol::type::string:
                std::cout << l_obj.as<std::string>();
                break;

                case sol::type::boolean:
                std::cout << l_obj.as<bool>() ? "True" : "False";
                break;

                case sol::type::number:
                std::cout << l_obj.as<float>();
                break;

                case sol::type::nil:
                std::cout << "Nil";
                break;

                case sol::type::function:
                std::cout << "Function";
                break;

                case sol::type::table:
                std::cout << "Table";
                Table(l_obj.as<sol::table>());
                break;

                default:
                std::cout << "Type is not known";
                break;
            }

            l_obj = l_pair.second;
            std::cout << " Value: ";
            switch(l_obj.get_type())
            {
                case sol::type::string:
                std::cout << l_obj.as<std::string>();
                break;

                case sol::type::boolean:
                std::cout << l_obj.as<bool>() ? "True" : "False";
                break;

                case sol::type::number:
                std::cout << l_obj.as<float>();
                break;
                
                case sol::type::nil:
                std::cout << "Nil";
                break;

                case sol::type::function:
                std::cout << "Function";
                break;

                case sol::type::table:
                std::cout << "Table\n";
                Table(l_obj.as<sol::table>());
                break;

                default:
                std::cout << "Type is not known";
                break;
            }

            std::cout << std::endl;
        }
    }

    void DefineLuaLib()
    {
        auto l_context = Core::LuaContext::Instance().lock();

        LogTable = l_context->CreateTable();

        LogTable.set_function("Message", [](std::string _message)
                              {
                                printf("Recieved lua Log.Message: %s\n", _message.c_str());
                                LogManager->LogMessage(Log::MessageContent::NORMAL, _message);
                              });


        LogTable.set_function("Debug", [](const std::string& _message)
                              { LogManager->LogMessage(Log::MessageContent::DEBUG, _message); });
        LogTable.set_function("Warning", [](const std::string& _message)
                              { LogManager->LogMessage(Log::MessageContent::WARNING, _message); });
        LogTable.set_function("Error", [](const std::string& _message)
                              { LogManager->LogMessage(Log::MessageContent::ERROR, _message); });

        
        LogTable.set_function("Table", [](const sol::table _table)
                              { LogManager->LogMessage(Log::MessageContent::NORMAL, Table(_table)); });


        l_context->SetGlobal("Log", LogTable);
    }

    void Init()
    {
        LogManager = Log::Manager::Init();
    }

    void Stop()
    {
        LogTable.clear();
        LogManager->Stop();
    }
}