#ifndef CATEGORYFACTORY_H
#define CATEGORYFACTORY_H

#include <memory>
#include <unordered_map>
#include <bitset>
#include <vector>

#include "sol/sol.hpp"

namespace RanakEngine
{
namespace Core
{
    class LuaContext;
    class Category;
    class CategoryFactory
    {
        friend LuaContext;
        private:
        int m_size;

        // These are unordered as I do not care to specify less than (operator<) overloads for bitset and string
        std::unordered_map<std::bitset<1024>, std::shared_ptr<Category>> m_signatureToCategory;
        std::unordered_map<std::string, std::bitset<1024>> m_nameToSignature;

        std::weak_ptr<Category> RegisterCategory(sol::table _definitionTable);
        std::weak_ptr<Category> RegisterCategory(Category _category);
        public:
        CategoryFactory();
        ~CategoryFactory();

        std::weak_ptr<Category> GetByName(std::string _name);
        std::weak_ptr<Category> GetBySignature(std::bitset<1024> _signature);
        std::vector<std::weak_ptr<Category>> GetCategories(std::bitset<1024> _combinedSignature);
        void RenameCategory(std::bitset<1024> _signature, std::string _name);

    };
}
}

#endif