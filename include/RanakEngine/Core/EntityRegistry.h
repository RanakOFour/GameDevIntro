#ifndef ENTITYREGISTRY_H
#define ENTITYREGISTRY_H

#include <vector>
#include <map>
#include <bitset>
#include <memory>

#include "sol/sol.hpp"

namespace RanakEngine
{
    class LuaContext;
namespace Core
{
    class Category;
    class EntityRegistry
    {
        private:
        std::shared_ptr<LuaContext> m_luaContext;

        int m_nextFreeId;
        std::vector<int> m_idsToDelete;
        std::vector<int> m_freeIds;
        std::map<std::bitset<1024>, std::shared_ptr<Category>> m_categories;
        std::map<int, std::bitset<1024>> m_entityBitset;
        
        sol::table m_entityTable;

        void DeleteFlaggedEntities();
        public:
        EntityRegistry();
        ~EntityRegistry();

        int AddEntity();
        void RemoveEntity(int _id);

        void AddToCategory(int _id, std::bitset<1024> _signature);
        void AddToCategory(int _id, std::string _categoryName);

        void RemoveFromCategory(int _id, std::bitset<1024> _signature);
        void RemoveFromCategory(int _id, std::string _categoryName);

        std::weak_ptr<Category> GetCategory(std::bitset<1024> _signature);
        std::vector<int> GetEntitiesWith(std::bitset<1024> _combinedSignature);

        bool Empty();
        int GetEntityCount();
        std::vector<int> GetEntityIDs();
        std::vector<Category> GetAllCategories();
        sol::table& GetTable();
    };
}
}

#endif