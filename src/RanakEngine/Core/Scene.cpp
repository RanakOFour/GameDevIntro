#include "RanakEngine/Core/Scene.h"
#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Core/CategoryFactory.h"
#include "RanakEngine/Core/Category.h"

#include "RanakEngine/Log.h"

namespace RanakEngine::Core
{
    Scene::Scene()
        : m_name("Scene"), m_registry(), m_rules()
    {
        auto l_luaContext = LuaContext::Instance().lock();
        m_sceneTable = l_luaContext->CreateTable();
        m_sceneTable.set("Entities", m_registry.GetEntityTable());
        m_sceneTable.set("Categories", m_registry.GetCategoryTable());
        m_sceneTable.set("Rules", l_luaContext->CreateTable());
    }

    Scene::Scene(sol::table _tableData)
        : m_name(_tableData["name"]), m_registry(), m_rules(), m_sceneTable(_tableData)
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
        std::vector<int> l_entities = m_registry.GetEntitiesWith(_signature);

        for (int l_entity : l_entities)
        {
            m_registry.RemoveFromCategory(l_entity, _signature);
        }
    }

    void Scene::AddRule(Rule &_rule)
    {
        m_rules.push_back(_rule);
        m_sceneTable.raw_get<sol::table>("Rules").raw_set(_rule.GetName(), _rule);
    }

    void Scene::RemoveRule(Rule &_rule)
    {
        std::string l_name = _rule.GetName();
        for (int i = 0; i < m_rules.size(); i++)
        {
            if (m_rules[i].GetName() == l_name)
            {
                m_rules.erase(m_rules.begin() + i);
                break;
            }
        }

        m_sceneTable.raw_get<sol::table>("Rules").raw_set(_rule.GetName(), sol::lua_nil);
    }

    void Scene::Init()
    {
        auto l_contextPtr = m_luaContext.lock();

        for (Rule& l_rule : m_rules)
        {
            l_rule.CreateSignature();
            l_rule.Init(m_registry);
        }
    }

    void Scene::Update(float _dt)
    {
        auto l_contextPtr = m_luaContext.lock();

        for (Rule& l_rule : m_rules)
        {
            l_rule.Update(m_registry);
        }
    }

    void Scene::Draw()
    {
        auto l_contextPtr = m_luaContext.lock();

        for(Rule& l_rule : m_rules)
        {
            l_rule.Draw(m_registry);
        }
    }

    EntityRegistry* Scene::GetRegistry()
    {
        return &m_registry;
    }

    sol::table Scene::GetSceneTable()
    {
        return m_sceneTable;
    }
}