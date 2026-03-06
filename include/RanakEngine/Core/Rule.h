#ifndef RULE_H
#define RULE_H

#include <string>
#include <vector>
#include <memory>

#include "sol/sol.hpp"

namespace RanakEngine::Resources
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
        Rule(std::weak_ptr<Resources::LuaFile> _file);

        ~Rule();

        void Update(float _dt, LuaContext* _context);
        void Draw(LuaContext* _context);

        std::string GetName();
        std::vector<std::string> GetCategories();
        sol::table& GetData();
    };
}

#endif