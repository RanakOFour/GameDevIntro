#ifndef RANAKLUA_H
#define RANAKLUA_H

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

        LuaContext();

        public:
        ~LuaContext();

        static std::shared_ptr<LuaContext> Init();
        static std::weak_ptr<LuaContext> Instance();

        void LoadScript(std::string _filePath);
        void RunScript(std::string _filePath);

        sol::table CreateTable();

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