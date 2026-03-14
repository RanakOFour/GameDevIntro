#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <vector>
#include <memory>

#include "RanakEngine/Core/EntityRegistry.h"
#include "RanakEngine/Core/CategoryFactory.h"
#include "RanakEngine/Core/Rule.h"
#include "RanakEngine/Asset/LuaFile.h"
#include "RanakEngine/Core/LuaContext.h"
#include "sol/sol.hpp"

namespace RanakEngine
{
    class LuaContext;

namespace Core
{
    class Camera;
    class Scene
    {
        private:
        std::weak_ptr<LuaContext> m_luaContext;

        std::string m_name;
        sol::table m_sceneTable;

        std::weak_ptr<Camera> m_camera;
        
        EntityRegistry m_registry;

        std::vector<Rule> m_rules;

        static void DefineUsertype()
        {
            auto l_context = LuaContext::Instance().lock();
            l_context->AddUserType<Scene>("Scene", sol::constructors<Scene(), Scene(sol::table)>(),
                                          "name", sol::readonly(&Scene::m_name),
                                          "camera", &Scene::m_camera,
                                          "registry", sol::readonly(&Scene::m_sceneTable),
                                          "rules", sol::readonly(&Scene::m_rules)
                                         );
        };

        public:
        Scene();
        Scene(sol::table _tableData);
        ~Scene();

        int AddEntity();
        int AddEntity(std::bitset<1024> _signature);
        void AddEntityToCategory(int _id, std::bitset<1024> _bitset);
        void RemoveEntity(int _id);

        void RemoveCategory(std::bitset<1024> _signature);

        void AddRule(Rule& _rule);
        void RemoveRule(Rule& _rule);

        void Init();
        void Update(float _deltaTime);
        void Draw();

        EntityRegistry* GetRegistry();
        sol::table GetSceneTable();
    };
};
};

#endif