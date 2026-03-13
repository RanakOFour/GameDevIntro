#ifndef RULE_H
#define RULE_H

#include <string>
#include <vector>
#include <memory>

#include "RanakEngine/Core/EntityRegistry.h"

#include "sol/sol.hpp"

namespace RanakEngine::Asset
{
    class LuaFile;
}

namespace RanakEngine::Core
{
    class LuaContext;
    class Scene;
    class Rule
    {
        friend Scene;
        friend LuaContext;
        private:
        std::weak_ptr<LuaContext> m_context;

        std::string m_name;
        std::bitset<1024> m_signature;
        std::vector<std::string> m_categories;
        sol::function m_initFunction;
        sol::function m_updateFunction;
        sol::function m_drawFunction;
        sol::table m_table;

        static void DefineUsertype(sol::state& _state)
        {
            _state.new_usertype<Rule>("Rule", sol::constructors<Rule()>(),
                                              "name", &Rule::m_name,
                                              "categories", &Rule::m_categories,
                                              "Init", &Rule::m_initFunction,
                                              "Update", &Rule::m_updateFunction,
                                              "Draw", &Rule::m_drawFunction,
                                              "data", &Rule::m_table,
                                              "getCategories", sol::readonly(&Rule::GetCategories)
                                     );
        }

        void CreateSignature();

        public:
        Rule();
        ~Rule();

        /* These will call the stand-in functions inside the m_table,
         passing in each entities data to the lua function 1 by one.
         This way, the lua function will have the header:
         Rule:Update(_entityData),
         Where _entityData is a table containing a single entities data.
         I should also probably hold the dt value somewhere else, too
         */
        void Init(EntityRegistry& _registry);
        void Update(EntityRegistry& _registry);
        void Draw(EntityRegistry& _registry);

        std::string GetName();
        std::bitset<1024> GetCategories();
        sol::table GetData();
    };
}

#endif