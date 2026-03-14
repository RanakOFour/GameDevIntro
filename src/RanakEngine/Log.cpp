#include "RanakEngine/Log.h"
#include "RanakEngine/Core/LuaContext.h"
#include "sol/sol.hpp"

namespace RanakEngine::Log
{
    namespace
    {
        static sol::table LogTable;
    };

    std::string Table(sol::table _table, int _tabIndex = 0)
    {
        std::string l_toReturn = "";
        printf("Printing table data:\n");
        auto l_tablePairs = _table.pairs();

        // Iterate key-value pairs of the table
        for(auto l_pair : l_tablePairs)
        {
            for(int i = 0; i < _tabIndex; i++)
            {
                l_toReturn += ' ';
            }

            sol::object l_obj = l_pair.first;
            l_toReturn += "Entry: ";
            switch(l_obj.get_type())
            {
                case sol::type::string:
                l_toReturn += l_obj.as<std::string>();
                break;

                case sol::type::boolean:
                l_toReturn += l_obj.as<bool>() ? "True" : "False";
                break;

                case sol::type::number:
                l_toReturn += l_obj.as<float>();
                break;

                case sol::type::nil:
                l_toReturn += "Nil";
                break;

                case sol::type::function:
                l_toReturn += "Function";
                break;

                case sol::type::table:
                l_toReturn += "Table";
                l_toReturn += Table(l_obj.as<sol::table>(), _tabIndex + 1);
                break;

                default:
                l_toReturn += "Type is not known";
                break;
            }

            l_obj = l_pair.second;
            l_toReturn += " Value: ";
            switch(l_obj.get_type())
            {
                case sol::type::string:
                l_toReturn += l_obj.as<std::string>();
                break;

                case sol::type::boolean:
                l_toReturn += l_obj.as<bool>() ? "True" : "False";
                break;

                case sol::type::number:
                l_toReturn += l_obj.as<float>();
                break;
                
                case sol::type::nil:
                l_toReturn += "Nil";
                break;

                case sol::type::function:
                l_toReturn += "Function";
                break;

                case sol::type::table:
                l_toReturn += "Table\n";
                l_toReturn += Table(l_obj.as<sol::table>(), _tabIndex + 1);
                break;

                default:
                l_toReturn += "Type is not known";
                break;
            }

            l_toReturn += "\n";
        }

        return l_toReturn;
    }

    void DefineLuaLib()
    {
        auto l_context = Core::LuaContext::Instance().lock();

        LogTable = l_context->CreateTable();

        LogTable.set_function("Message", [](const std::string _message)
                              {
                                LogManager->LogMessage(Log::MessageContent::NORMAL, _message);
                              });
        LogTable.set_function("Debug", [](const std::string _message)
                              { LogManager->LogMessage(Log::MessageContent::DEBUG, _message); });
        LogTable.set_function("Warning", [](const std::string _message)
                              { LogManager->LogMessage(Log::MessageContent::WARNING, _message); });
        LogTable.set_function("Error", [](const std::string _message)
                              { LogManager->LogMessage(Log::MessageContent::ERROR, _message); });
        LogTable.set_function("Table", [](const sol::table _table)
                              { LogManager->LogMessage(Log::MessageContent::NORMAL, Table(_table, 0)); });


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