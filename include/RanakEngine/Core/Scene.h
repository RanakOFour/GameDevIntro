#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <vector>
#include <memory>

#include "RanakEngine/Core/EntityRegistry.h"
#include "RanakEngine/Core/CategoryFactory.h"
#include "RanakEngine/Core/Rule.h"
#include "RanakEngine/Resources/LuaFile.h"
#include "sol/sol.hpp"

namespace RanakEngine
{
    class LuaContext;

namespace Core
{
    class Scene
    {
        private:
        std::weak_ptr<LuaContext> m_luaContext;

        std::string m_name;
        sol::table m_sceneTable;
        
        EntityRegistry m_registry;

        std::vector<Rule> m_rules;

        void DeserializeFromLua(sol::table _tableData);
        void ConfigLuaLibrary();

        public:
        Scene();
        Scene(sol::table _tableData);
        ~Scene();

        int AddEntity();
        int AddEntity(std::bitset<1024> _signature);
        void AddEntityToCategory(int _id, std::bitset<1024> _bitset);
        void RemoveEntity(int _id);

        void AddCategory(std::weak_ptr<Category> _file);

        void RemoveCategory(std::bitset<1024> _signature);

        void AddRule(Rule& _rule);
        void RemoveRule();

        void Update(float _deltaTime);
        void Draw();

    };
};
};

#endif