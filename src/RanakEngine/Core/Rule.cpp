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
        m_table = LuaContext::Instance().lock()->CreateTable();
    }
}