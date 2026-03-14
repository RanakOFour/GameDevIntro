#ifndef CAMERA_H
#define CAMERA_H

#include <memory>

#include "sol/sol.hpp"
#include "GLM/ext.hpp"
#include "RanakEngine/Math.h"
#include "RanakEngine/Core/LuaContext.h"
#include "RanakEngine/Asset/Shader.h"

namespace RanakEngine::Core
{
    class Camera
    {
        friend LuaContext;
        public:
        enum ProjectionType
        {
            Perspective,
            Orthographic
        };

        private:
        bool m_isLookingAt;
        float m_lookAtDistance;
        int m_lookAtTarget;

        Vector3 m_position;
        float m_rotation;

        float m_fov;

        ProjectionType m_projectionType;
        glm::mat4 m_projection;

        static void DefineUsertype(sol::state& _state)
        {
            _state.new_usertype<Camera>("Camera",
                                        "position", &Camera::m_position,
                                        "rotation", &Camera::m_rotation,
                                        "fov", &Camera::m_fov,
                                        "Draw", &Camera::Draw,
                                        "setLookAtTarget", &Camera::SetLookAtTarget,
                                        "getLookAtTarget", &Camera::GetLookAtTarget,
                                        "setLookAtDistance", &Camera::SetLookAtDistance,
                                        "getLookAtDistance", &Camera::GetLookAtDistance,
                                        "setProjectionType", &Camera::SetProjectionType,
                                        "getProjectionType", &Camera::GetProjectionType);
        }

        public:
        Camera();
        ~Camera();

        void Use(std::shared_ptr<Asset::Shader> _shader);

        void Draw(sol::table _transform, sol::table _drawable);

        void SetPosition(Vector3 _pos);

        void SetLookAtTarget(int _id);
        int GetLookAtTarget();

        void SetLookAtDistance(float _d);
        float GetLookAtDistance();

        void SetProjectionType(int _t);
        ProjectionType GetProjectionType();
    };
}

#endif