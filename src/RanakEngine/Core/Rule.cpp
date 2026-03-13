#include "RanakEngine/Core/Rule.h"
#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Core/EntityRegistry.h"
#include "RanakEngine/Core/Category.h"
#include "RanakEngine/Log.h"
#include "RanakEngine/Math.h"

namespace RanakEngine::Core
{
    void Rule::CreateSignature()
    {
        auto l_context = LuaContext::Instance().lock();

        for(std::string l_categoryName : m_categories)
        {
            std::bitset<1024> l_signature = l_context->GetCategory(l_categoryName).lock()->GetSignature();
            m_signature |= l_signature;
        }
    }

    Rule::Rule()
    : m_updateFunction()
    , m_initFunction()
    {
        m_table = LuaContext::Instance().lock()->CreateTable();
        m_context = LuaContext::Instance();
    }

    // Rule::Rule(sol::table _dataTable)
    // : m_signature()
    // {
    //     m_context = LuaContext::Instance();
    //     auto l_context = m_context.lock();

    //     // Create signature
    //     auto l_categoryNames = m_table.raw_get<sol::table>("categories").pairs();

    //     for(auto l_namePair : l_categoryNames)
    //     {
    //     }
    // }

    Rule::~Rule()
    {

    }

    void Rule::Init(EntityRegistry& _registry)
    {
        if(!m_initFunction.valid())
        {
            return;
        }

        // Get entities that match signature
        std::vector<int> l_entities = _registry.GetEntitiesWith(m_signature);

        // Call update function passing in each entity's data
        if(l_entities.size() > 0)
        {
            for(int l_entity: l_entities)
            {
                sol::table l_entityData = _registry.GetEntityAttributes(l_entity);
                m_initFunction(this, l_entityData);
            }
        }
    }

    void Rule::Update(EntityRegistry& _registry)
    {
       // Early quit for no function found/defined
        if(!m_updateFunction.valid())
        {
            return;
        }

        // Get entities that match signature
        std::vector<int> l_entities = _registry.GetEntitiesWith(m_signature);

        // Call update function passing in each entity's data
        if(l_entities.size() > 0)
        {
            for(int l_entity: l_entities)
            {
                sol::table l_entityData = _registry.GetEntityAttributes(l_entity);
                m_updateFunction(this, l_entityData);
            }
        }
    }

    void Rule::Draw(EntityRegistry& _registry)
    {
        // Early quit for no function found/defined
        if(!m_drawFunction.valid())
        {
            return;
        }

        std::vector<int> l_entities = _registry.GetEntitiesWith(m_signature);

        // Call update function passing in each entity's data
        if(l_entities.size() > 0)
        {
            for(int l_entity: l_entities)
            {
                sol::table l_entityData = _registry.GetEntityAttributes(l_entity);
                m_drawFunction(this, l_entityData);
            }
        }
    }

    std::string Rule::GetName()
    {
        return m_name;
    }

    std::bitset<1024> Rule::GetCategories()
    {
        return m_signature;
    }

    sol::table Rule::GetData()
    {
        return m_table;
    }
}