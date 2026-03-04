#ifndef CATEGORY_H
#define CATEGORY_H

#include "sol/sol.hpp"

#include <string>
#include <bitset>
#include <vector>
#include <map>

namespace RanakEngine::Core
{
    class EntityRegistry;
    class Category
    {
        private:
        std::string m_name;
        std::bitset<1024> m_signature;
        sol::table m_baseAttributeTable;

        int m_size;
        std::vector<sol::table> m_entityDataTables;
        std::map<int, int> m_entityToIndex;
        std::map<int, int> m_indexToEntity;

        public:
        Category(sol::table _baseAttributes);
        ~Category();

        void AddMember(int _id, EntityRegistry& _registry);
        void RemoveMember(int id, EntityRegistry& _registry);

        std::bitset<1024> GetSignature();
    };
}

#endif