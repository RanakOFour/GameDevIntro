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
    class Category;
    class Rule
    {
        friend LuaContext;
        private:
        std::weak_ptr<LuaContext> m_context;

        std::string m_name;
        std::vector<std::weak_ptr<Category>> m_categories;
        std::bitset<1024> m_signature;
        sol::table m_table;

        static void DefineUsertype(sol::state& _state)
        {
            _state.new_usertype<Rule>("Rule", "name", sol::readonly(&Rule::m_name),
                                              "getCategories", sol::readonly(&Rule::GetCategories)
                                     );
        }

        public:
        Rule();
        Rule(std::weak_ptr<Asset::LuaFile> _file);

        ~Rule();

        void Update(float _dt, EntityRegistry& _registry);
        void Draw(EntityRegistry& _registry);

        std::string GetName();
        std::bitset<1024> GetCategories();
        sol::table GetData();
    };
}

#endif