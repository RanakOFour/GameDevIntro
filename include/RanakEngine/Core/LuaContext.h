#ifndef RANAKLUA_H
#define RANAKLUA_H

#include "RanakEngine/Asset/LuaFile.h"
#include "RanakEngine/Core/CategoryFactory.h"

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

#include <memory>
#include <map>

namespace RanakEngine::Core
{
    class LuaContext
    {
        private:
        inline static std::weak_ptr<LuaContext> m_self;
        sol::state m_state;
        std::map<std::string, sol::load_result> m_loadedScripts;
        std::shared_ptr<Core::CategoryFactory> m_categoryFactory;

        LuaContext();

        public:
        ~LuaContext();

        static std::shared_ptr<LuaContext> Init();
        static std::weak_ptr<LuaContext> Instance();

        void LoadScript(std::weak_ptr<Asset::LuaFile> _file);

        template<typename T>
        T RunScript(std::weak_ptr<Asset::LuaFile> _file)
        {
            auto l_file = _file.lock();
            std::string l_path = l_file->GetPath();
            std::string l_code = l_file->GetCode();

            if(m_loadedScripts.find(l_path) == m_loadedScripts.end())
            {
                LoadScript(_file);
            }

            sol::protected_function_result l_result = m_loadedScripts[l_path]();

            if(!l_result.valid())
            {
                sol::error l_err = l_result;
                printf("Error running script \"%s\":\n%s\n", l_path.c_str(), l_err.what());
            }

            // Return on void
            if constexpr (std::is_void<T>::value)
            {
                return;
            }

            // Get result type of there is a result
            T l_toReturn = l_result.get<T>();
            return l_toReturn;
        };

        sol::table CreateTable();

        std::weak_ptr<Core::Category> CreateCategory(std::weak_ptr<Asset::LuaFile> _file);
        std::weak_ptr<Core::Category> GetCategory(std::bitset<1024> _signature);

        template<typename T>
        void AddVariable(std::string _name, T& _var)
        {
            m_state[_name] = &_var;
        };

        template<typename T, typename... Args>
        void AddUserType(Args&&... _args)
        {
            m_state.new_usertype<T>(std::forward(_args)...);
        }

        sol::state* GetState() { return &m_state; };
    };
}

#endif