#include "RanakEngine/Core/Scene.h"
#include "RanakEngine/LuaContext.h"
#include "RanakEngine/Core/CategoryFactory.h"
#include "RanakEngine/Core/Category.h"

#include "RanakEngine/Log.h"

namespace RanakEngine::Core
{
    Core::Scene::Scene()
    : m_name("Scene")
    , m_registry()
    , m_rules()
    {
        auto l_luaContext = LuaContext::Instance().lock();
        m_sceneTable = l_luaContext->CreateTable();
        l_luaContext->AddVariable<sol::table>("Scene", m_sceneTable);
        m_sceneTable.set("Registry", m_registry.GetTable());
    }

    Core::Scene::Scene(sol::table _tableData)
    : m_name(_tableData["name"])
    , m_registry()
    , m_rules()
    , m_sceneTable(_tableData)
    {
        auto l_luaContext = LuaContext::Instance().lock();
        l_luaContext->AddVariable<sol::table>("Scene", m_sceneTable);
    }

    int Core::Scene::AddEntity()
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

    void Scene::AddCategory(std::weak_ptr<Category> _category)
    {
        auto l_newCategory = _category.lock();
        m_sceneTable.get<sol::table>("Categories").add(l_newCategory->GetName(), l_newCategory->GetBaseData());
    }
}