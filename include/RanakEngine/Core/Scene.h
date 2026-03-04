#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <vector>

#include "RanakEngine/Core/EntityRegistry.h"
#include "sol/sol.hpp"

namespace RanakEngine::Core
{
    class EntityRegistry;
    class Rule;
    class Scene
    {
        std::string m_name;
        sol::table m_sceneTable;
        EntityRegistry m_registry;
        std::vector<Rule> m_rules;
    };
}

#endif