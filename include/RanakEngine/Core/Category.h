#ifndef CATEGORY_H
#define CATEGORY_H

#include "sol/sol.hpp"

#include <string>
#include <bitset>
#include <vector>
#include <map>

namespace RanakEngine::Core
{
    class LuaContext;
    class CategoryFactory;
    class EntityRegistry;
    class Category
    {
        friend LuaContext;
        friend CategoryFactory;
        private:
        std::string m_name;
        std::bitset<1024> m_signature;
        sol::table m_baseAttributeTable;

        int m_size;
        std::vector<sol::table> m_entityDataTables;
        std::map<int, int> m_entityToIndex;
        std::map<int, int> m_indexToEntity;

        static void DefineUsertype(sol::state& _state)
        {
            _state.new_usertype<Category>("Category", "name", sol::readonly(&Category::m_name),
                                                      "attributes", sol::readonly(&Category::m_baseAttributeTable),
                                                      "getMembers", sol::readonly(&Category::GetMembers),
                                                      "getDataFor", &Category::GetDataFor,
                                                      sol::constructors<Category()>());
        };

        std::vector<int> GetMembers()
        {
            std::vector<int> l_toReturn;
            for(auto l_pair : m_entityToIndex)
            {
                l_toReturn.push_back(l_pair.first);
            }

            return l_toReturn;
        }

        public:
        Category();
        Category(std::string _name, sol::table _baseAttributes);
        ~Category();

        sol::table& AddMember(int _id);
        void RemoveMember(int id);

        sol::table& GetBaseData();
        sol::table GetDataFor(int _id);
        std::string GetName();
        int GetSize();
        std::bitset<1024> GetSignature();
    };
}

#endif