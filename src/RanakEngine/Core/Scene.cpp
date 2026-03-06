#include "RanakEngine/Core/Scene.h"
#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Core/CategoryFactory.h"
#include "RanakEngine/Core/Category.h"

#include "RanakEngine/Log.h"

namespace RanakEngine::Core
{
    Scene::Scene()
    : m_name("Scene")
    , m_registry()
    , m_rules()
    {
        auto l_luaContext = LuaContext::Instance().lock();
        m_sceneTable = l_luaContext->CreateTable();
        l_luaContext->AddVariable<sol::table>("Scene", m_sceneTable);
        m_sceneTable.set("Entities", m_registry.GetEntityTable());
        m_sceneTable.set("Categories", m_registry.GetCategoryTable());
    }

    Scene::Scene(sol::table _tableData)
    : m_name(_tableData["name"])
    , m_registry()
    , m_rules()
    , m_sceneTable(_tableData)
    {
        auto l_luaContext = LuaContext::Instance().lock();
        l_luaContext->AddVariable<sol::table>("Scene", m_sceneTable);
    }

    Scene::~Scene()
    {
        
    }

    int Scene::AddEntity()
    {
        return m_registry.AddEntity();
    }

    int Scene::AddEntity(std::bitset<1024> _signature)
    {
        int l_newEntityID = m_registry.AddEntity();
        m_registry.AddToCategory(l_newEntityID, _signature);

        return l_newEntityID;
    }

    void Scene::AddEntityToCategory(int _id, std::bitset<1024> _signature)
    {
        m_registry.AddToCategory(_id, _signature);
    }

    void Scene::RemoveEntity(int _id)
    {
        m_registry.RemoveEntity(_id);
    }

    void Scene::RemoveCategory(std::bitset<1024> _signature)
    {
        m_registry.RemoveCategory(_signature);
    }

    void Scene::AddRule(Rule& _rule)
    {
        m_rules.push_back(_rule);
    }

    void Scene::RemoveRule(Rule& _rule)
    {
        std::string l_name = _rule.GetName();
        for(int i = 0; i < m_rules.size(); i++)
        {
            if(m_rules[i].GetName() == l_name)
            {
                m_rules.erase(m_rules.begin() + i);
                break;
            }
        }
    }

    void Scene::Update(float _dt)
    {
        auto l_contextPtr = m_luaContext.lock();

        for(Rule& l_rule : m_rules)
        {
            l_rule.Update(_dt, l_contextPtr.get());
        }
    }

    void Scene::Draw()
    {
        auto l_contextPtr = m_luaContext.lock();

        for(Rule& l_rule : m_rules)
        {
            l_rule.Draw(l_contextPtr.get());
        }
    }
}