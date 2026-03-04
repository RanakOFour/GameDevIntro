#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <vector>
#include <memory>

#include "RanakEngine/Core/EntityRegistry.h"
#include "RanakEngine/Resources/LuaFile.h"
#include "sol/sol.hpp"

namespace RanakEngine::Core
{
    class EntityRegistry;
    class Rule;
    class Scene
    {
        private:
        std::string m_name;
        sol::table m_sceneTable;
        EntityRegistry m_registry;
        std::vector<Rule> m_rules;

        void DeserializeFromLua(sol::table _tableData);
        void ConfigLuaLibrary();

        public:
        Scene();
        Scene(std::weak_ptr<Resources::LuaFile> _file);
        Scene(sol::table _tableData);
        ~Scene();

        int AddEntity();
        int AddEntity(std::bitset<1024> _signature);
        void RemoveEntity(int _id);

        void AddCategory(std::weak_ptr<Category> _category);
        void RemoveCategory(std::bitset<1024> _signature);

        void AddRule(Rule& _rule);
        void RemoveRule();

        void Update(float _deltaTime);
        void Draw();

    };
};

#endif