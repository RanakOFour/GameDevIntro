#include "RanakEngine/Core/Category.h"
#include "RanakEngine/Core/LuaContext.h"

namespace RanakEngine::Core
{
    Category::Category()
    : m_name()
    , m_signature()
    , m_size(0)
    , m_entityDataTables()
    , m_entityToIndex()
    , m_indexToEntity()
    {
        m_baseAttributeTable = LuaContext::Instance().lock()->CreateTable();
    }

    Category::Category(std::string _name, sol::table _baseAttributes)
    : m_name(_name)
    , m_signature()
    , m_size(0)
    , m_entityDataTables()
    , m_entityToIndex()
    , m_indexToEntity()
    {
        m_baseAttributeTable = _baseAttributes;
    }

    Category::~Category()
    {

    }

    void CloneTable(sol::table& _toCopy, sol::table& _target, LuaContext* _context)
    {
        auto l_copyPairs = _toCopy.pairs();
        for(auto l_pair : l_copyPairs)
        {
            if(l_pair.second.get_type() == sol::type::table)
            {
                sol::table l_newCopyTarget = l_pair.second.as<sol::table>();
                _target[l_pair.first] = _context->CreateTable();
                sol::table l_newTarget = _target.raw_get<sol::table>(l_pair.first);
                CloneTable(l_newCopyTarget, l_newTarget, _context);
            }
            else
            {
                _target[l_pair.first] = l_pair.second;
            }
        }
    }

    sol::table& Category::AddMember(int _id)
    {
        auto l_luaContext = LuaContext::Instance().lock();
        
        // Create new table for entity
        sol::table l_newTable = l_luaContext->CreateTable();
        CloneTable(m_baseAttributeTable, l_newTable, l_luaContext.get());

        m_entityDataTables.push_back(l_newTable);
        m_entityToIndex[_id] = m_size;
        m_indexToEntity[m_size] = _id;
        m_size++;

        return m_entityDataTables.back();
    }

    void Category::RemoveMember(int _id)
    {
        auto l_entToInd = m_entityToIndex.find(_id);
        // Can't find ID
        if(l_entToInd == m_entityToIndex.end())
        {
            return;
        }

        // Get indexes to delete
        int l_indexToRemove = l_entToInd->second;
        int l_last = m_size - 1;

        sol::table l_tableData = m_entityDataTables[l_indexToRemove];

        // Shift ids into the back of the maps
        m_entityDataTables[l_indexToRemove] = m_entityDataTables[l_last];

        // Replace the old position of the ids with the data at the back to preserve contiguousness
        int l_entityInLast = m_indexToEntity[l_last];
        m_entityToIndex[l_entityInLast] = l_indexToRemove;
        m_indexToEntity[l_indexToRemove] = l_entityInLast;

        m_entityToIndex.erase(_id);
        m_indexToEntity.erase(l_last);

        m_entityDataTables.pop_back();

        m_size--;
    }

    sol::table& Category::GetBaseData()
    {
        return m_baseAttributeTable;
    }

    sol::table Category::GetAttributesFor(int _id)
    {
        if(m_entityToIndex.find(_id) == m_entityToIndex.end())
        {
            return nullptr;
        }

        return m_entityDataTables[m_entityToIndex[_id]];
    }

    std::vector<int> Category::GetMembers()
    {
        std::vector<int> l_toReturn;
        for(auto l_pair : m_entityToIndex)
        {
            l_toReturn.push_back(l_pair.first);
        }

        return l_toReturn;
    }

    std::string Category::GetName()
    {
        return m_name;
    }

    int Category::GetSize()
    {
        return m_size;
    }

    std::bitset<1024> Category::GetSignature()
    {
        return m_signature;
    }
}