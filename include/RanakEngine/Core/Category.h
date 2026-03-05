#ifndef CATEGORY_H
#define CATEGORY_H

#include "sol/sol.hpp"

#include <string>
#include <bitset>
#include <vector>
#include <map>

namespace RanakEngine::Core
{
    class CategoryFactory;
    class EntityRegistry;
    class Category
    {
        friend CategoryFactory;
        private:
        std::string m_name;
        std::bitset<1024> m_signature;
        sol::table m_baseAttributeTable;

        int m_size;
        std::vector<sol::table> m_entityDataTables;
        std::map<int, int> m_entityToIndex;
        std::map<int, int> m_indexToEntity;

        public:
        Category();
        Category(std::string _name, sol::table _baseAttributes);
        ~Category();

        sol::table& AddMember(int _id);
        void RemoveMember(int id);

        sol::table& GetBaseData();
        sol::table GetDataFor(int _id);
        std::string GetName();
        std::bitset<1024> GetSignature();
    };
}

#endif