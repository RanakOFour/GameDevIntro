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
    class Category;
    class Rule
    {
        private:
        std::string m_name;
        std::vector<std::weak_ptr<Category>> m_categories;
        sol::table m_table;

        public:
        Rule();
        Rule(Resources::LuaFile _file);

        ~Rule();

        void Update(float _dt);
        void Draw();
    };
}

#endif