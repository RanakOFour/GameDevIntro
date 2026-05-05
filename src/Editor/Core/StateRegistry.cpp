#include "Editor/Core/StateRegistry.h"

#include "RanakEngine/RanakEngine.h"

void StateRegistry::RegisterCondition(const std::string& _name, std::function<bool()> _pred)
{
    m_conditions[_name] = std::move(_pred);
}

void StateRegistry::RegisterAction(const std::string& _name, std::function<void()> _action)
{
    m_actions[_name] = std::move(_action);
}

bool StateRegistry::Evaluate(const std::string& _name) const
{
    auto l_it = m_conditions.find(_name);
    if (l_it == m_conditions.end())
    {
        RE::Log::Warning("StateRegistry: unknown condition \"" + _name + "\"");
        return false;
    }
    return l_it->second();
}

bool StateRegistry::Apply(const std::string& _name) const
{
    auto l_it = m_actions.find(_name);
    if (l_it == m_actions.end())
    {
        RE::Log::Warning("StateRegistry: unknown action \"" + _name + "\"");
        return false;
    }
    l_it->second();
    return true;
}
