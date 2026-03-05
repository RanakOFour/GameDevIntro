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
        sol::table m_table;

        static void DefineUsertype(sol::state& _state)
        {
            _state.new_usertype<Rule>("Rule", "name", sol::readonly(&Rule::m_name),
                                              "getCategories", sol::readonly(&Rule::GetCategories),
                                              "Update", sol::readonly(&Rule::Update),
                                              "Draw", sol::readonly(&Rule::Draw));
        }

        public:
        Rule();
        Rule(std::weak_ptr<Resources::LuaFile> _file);

        ~Rule();

        void Update(float _dt);
        void Draw();

        std::string GetName();
        std::vector<std::string> GetCategories();
        sol::table& GetData();
    };
}

#endif