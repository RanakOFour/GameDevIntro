#include "RanakEngine/Core/Rule.h"
#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Core/EntityRegistry.h"
#include "RanakEngine/Core/Category.h"
#include "RanakEngine/Log.h"
#include "RanakEngine/Math.h"

void PrintTable(sol::table _table)
{
    printf("Printing table data:\n");
    auto l_tablePairs = _table.pairs();

    // Iterate key-value pairs of the table
    for(auto l_pair : l_tablePairs)
    {
        sol::object l_obj = l_pair.first;
        std::cout << "Entry: ";
        switch(l_obj.get_type())
        {
            case sol::type::string:
            std::cout << l_obj.as<std::string>();
            break;

            case sol::type::boolean:
            std::cout << l_obj.as<bool>() ? "True" : "False";
            break;

            case sol::type::number:
            std::cout << l_obj.as<float>();
            break;

            case sol::type::nil:
            std::cout << "Nil";
            break;

            case sol::type::function:
            std::cout << "Function";
            break;

            case sol::type::table:
            std::cout << "Table";
            PrintTable(l_obj.as<sol::table>());
            break;

            default:
            std::cout << "Type is not known";
            break;
        }

        l_obj = l_pair.second;
        std::cout << " Value: ";
        switch(l_obj.get_type())
        {
            case sol::type::string:
            std::cout << l_obj.as<std::string>();
            break;

            case sol::type::boolean:
            std::cout << l_obj.as<bool>() ? "True" : "False";
            break;

            case sol::type::number:
            std::cout << l_obj.as<float>();
            break;
            
            case sol::type::nil:
            std::cout << "Nil";
            break;

            case sol::type::function:
            std::cout << "Function";
            break;

            case sol::type::table:
            std::cout << "Table\n";
            PrintTable(l_obj.as<sol::table>());
            break;

            default:
            std::cout << "Type is not known";
            break;
        }

        std::cout << std::endl;
    }
}

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
    , m_drawFunction()
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
                PrintTable(l_entityData);
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