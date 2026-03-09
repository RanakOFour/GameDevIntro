#ifndef ENTITYREGISTRY_H
#define ENTITYREGISTRY_H

#include <vector>
#include <map>
#include <unordered_map>
#include <bitset>
#include <memory>

#include "sol/sol.hpp"

namespace RanakEngine::Core
{
    class LuaContext;
    class Category;
    class EntityRegistry
    {
        private:
        std::shared_ptr<LuaContext> m_luaContext;

        int m_nextFreeId;
        std::vector<int> m_idsToDelete;
        std::vector<int> m_freeIds;
        std::unordered_map<std::bitset<1024>, std::shared_ptr<Category>> m_categories;
        std::map<int, std::bitset<1024>> m_entityBitset;
        
        sol::table m_dataTable;

        void DeleteFlaggedEntities();
        public:
        EntityRegistry();
        ~EntityRegistry();

        int AddEntity();
        void RemoveEntity(int _id);

        void AddToCategory(int _id, std::bitset<1024> _signature);
        void RemoveFromCategory(int _id, std::bitset<1024> _signature);

        std::weak_ptr<Category> GetCategory(std::bitset<1024> _signature);
        std::vector<int> GetEntitiesWith(std::bitset<1024> _combinedSignature);
        sol::table GetEntityAttributes(int _id);

        bool Empty();
        int GetEntityCount();

        sol::table GetEntityTable();
        sol::table GetCategoryTable();
    };
}

#endif