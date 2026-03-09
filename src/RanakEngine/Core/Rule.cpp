#include "RanakEngine/Core/Rule.h"
#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Core/EntityRegistry.h"

namespace RanakEngine::Core
{
    Rule::Rule()
    {
        m_table = LuaContext::Instance().lock()->CreateTable();
        m_context = LuaContext::Instance();
    }

    Rule::Rule(sol::table _dataTable)
    : m_signature()
    {
        m_context = LuaContext::Instance();
        auto l_context = m_context.lock();

        // Create signature
        auto l_categoryNames = m_table.raw_get<sol::table>("categories").pairs();

        for(auto l_namePair : l_categoryNames)
        {
        }
    }

    Rule::Rule(std::weak_ptr<Asset::LuaFile> _file)
    {
        auto l_contextPtr = LuaContext::Instance().lock();
        m_table = l_contextPtr->RunScript<sol::table>(_file);
        m_context = LuaContext::Instance();
    }

    Rule::~Rule()
    {

    }

    void Rule::Update(EntityRegistry& _registry)
    {
        sol::protected_function l_updateFunction = m_table.raw_get<sol::function>("Update");
        

        // Early quit for no function found/defined
        if(!l_updateFunction.valid())
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
                l_updateFunction(l_entityData);
            }
        }
    }

    void Rule::Draw(EntityRegistry& _registry)
    {
        sol::protected_function l_drawFunction = m_table.raw_get<sol::function>("Draw");
        
        // Early quit for no function found/defined
        if(!l_drawFunction.valid())
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
                l_drawFunction(l_entityData);
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