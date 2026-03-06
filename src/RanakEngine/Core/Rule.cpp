#include "RanakEngine/Core/Rule.h"
#include "RanakEngine/Core/LuaContext.h"

namespace RanakEngine::Core
{
    Rule::Rule()
    : m_name()
    , m_categories()
    {
        m_table = LuaContext::Instance().lock()->CreateTable();
    }

    Rule::Rule(std::weak_ptr<Resources::LuaFile> _file)
    : m_name()
    , m_categories()
    {
        auto l_contextPtr = LuaContext::Instance().lock();
        m_table = l_contextPtr->RunScript<sol::table>(_file);
    }

    Rule::~Rule()
    {

    }

    void Rule::Update(float _dt, LuaContext* _context)
    {
        // Get entities that match signature

        // Call update function passing in each entity's data
    }
}