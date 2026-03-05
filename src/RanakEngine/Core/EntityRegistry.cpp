#include "RanakEngine/Core/EntityRegistry.h"
#include "RanakEngine/LuaContext.h"
#include "RanakEngine/Core/Category.h"

namespace RanakEngine::Core
{
    EntityRegistry::EntityRegistry()
    : m_nextFreeId(0)
    , m_freeIds()
    , m_categories()
    , m_entityBitset()
    {
        m_luaContext = LuaContext::Instance().lock();
        m_entityTable = m_luaContext->CreateTable();
    }

    EntityRegistry::~EntityRegistry()
    {

    }

    int EntityRegistry::AddEntity()
    {
        int l_newID{-1};

        if(!m_freeIds.empty())
        {
            l_newID = m_freeIds.back();
            m_freeIds.pop_back();
        }
        else
        {
            l_newID = ++m_nextFreeId;
        }

        m_entityTable.get<sol::table>("Entities")
                     .set(l_newID, m_luaContext->CreateTable());

        return l_newID;
    }

    void EntityRegistry::RemoveEntity(int _id)
    {
        m_idsToDelete.push_back(_id);

        // Clear table
        m_entityTable.get<sol::table>("Entities")
                     .get<sol::table>(_id)
                     .clear();
    }

    void EntityRegistry::DeleteFlaggedEntities()
    {
        for(int l_id : m_idsToDelete)
        {
            auto l_idSignature = m_entityBitset[l_id];

            for(auto l_pair : m_categories)
            {
                // If there is a match
                if((l_idSignature & l_pair.first).any())
                {
                    l_pair.second->RemoveMember(l_id);
                }
            }

            m_entityBitset.erase(l_id);

            m_freeIds.push_back(l_id);
        }

        m_freeIds.erase(m_freeIds.begin(), m_freeIds.end());
    }

    void EntityRegistry::AddToCategory(int _id, std::bitset<1024> _signature)
    {
        // Ask context for registered category
        std::shared_ptr<Category> l_categoryPtr = m_luaContext->GetCategory(_signature).lock();

        // Category is registered
        if(l_categoryPtr.get() != nullptr)
        {
            // Add to registry if not logged already
            if(m_categories.find(_signature) == m_categories.end())
            {
                // Add valid ptr to category map
                if(l_categoryPtr)
                {
                    m_categories[_signature] = l_categoryPtr;
                }
            }

            // Edit entityData in table
            m_entityTable.get<sol::table>("Entities")
                         .get<sol::table>(_id)
                         .get<sol::table>("attributes")
                         .set(l_categoryPtr->GetName(), l_categoryPtr->AddMember(_id)
                        );

            // Update entity signature
            m_entityBitset[_id] |= _signature;
        }
    }
}