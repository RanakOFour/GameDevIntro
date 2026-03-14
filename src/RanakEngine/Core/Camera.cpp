#include "RanakEngine/Core/Camera.h"
#include "RanakEngine/Assets.h"
#include "GL/glew.h"
#include "RanakEngine/Math.h"
#include "GLM/ext.hpp"

namespace RanakEngine::Core
{
    Camera::Camera()
    : m_isLookingAt(false)
    , m_lookAtDistance(3.0f)
    , m_lookAtTarget(-1)
    , m_position(0.0f, 0.0f, 3.0f)
    , m_rotation(0.0f)
    , m_fov(45.0f)
    , m_projectionType(ProjectionType::Perspective)
    {
        m_projection = glm::perspective(m_fov, 1920.0f/1080.0f, 0.1f, 100.0f);
    }

    Camera::~Camera()
    {

    }

    void Camera::Use(std::shared_ptr<Asset::Shader> _shader)
    {
        glm::vec3 l_glPos = glm::vec3(m_position.x, m_position.y, m_position.z);
        glm::quat l_glQuat = glm::angleAxis(m_rotation, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 translation = glm::translate(glm::mat4(1.0f), l_glPos);
        glm::mat4 rotation = glm::mat4(l_glQuat);
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
        
        glm::mat4 l_modelMat = translation * rotation * scale;

        //Logger::LogMessage(Message::DEBUG, "Transform::ModelMatrix()\n" + glm::to_string(l_modelMat));
        _shader->SetUniform("u_View", glm::inverse(l_modelMat));
        _shader->SetUniform("u_Projection", m_projection);
    }

    void Camera::Draw(sol::table _transform, sol::table _drawable)
    {
        Vector3& l_pos = _transform.raw_get<Vector3>("position");
        Vector3& l_euler = _transform.raw_get<Vector3>("rotation");

        glm::vec3 l_glPos = glm::vec3(l_pos.x, l_pos.y, l_pos.z);
        glm::quat l_glQuat = (glm::quat)Quaternion(l_euler.x, l_euler.y, l_euler.z);

        glm::mat4 translation = glm::translate(glm::mat4(1.0f), l_glPos);
        glm::mat4 rotation = glm::mat4(l_glQuat);
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
        
        glm::mat4 l_modelMat = translation * rotation * scale;

        std::string l_modelPath = _drawable.raw_get<std::string>("modelPath");
        if(l_modelPath == "")
        {
            l_modelPath = "./resources/models/FlatTexture.obj";
        }

        std::string l_texturePath = _drawable.raw_get<std::string>("texturePath");
        if(l_texturePath == "")
        {
            l_texturePath = "./resources/textures/triangle.png";
        }

        std::string l_shaderPath = _drawable.raw_get<std::string>("shaderPath");
        if(l_shaderPath == "")
        {
            l_shaderPath = "./resources/shaders/default/frag.fs;./resources/shaders/default/vert.vs";
        }

        auto l_model = Asset::Manager::Instance().lock()->Load<Asset::Model>(l_modelPath).lock();
        auto l_texture = Asset::Manager::Instance().lock()->Load<Asset::Texture>(l_texturePath).lock();
        auto l_shader = Asset::Manager::Instance().lock()->Load<Asset::Shader>(l_shaderPath).lock();
        
        l_shader->Use();
        
        glBindVertexArray(l_model->GetVAO());

        glBindTexture(GL_TEXTURE_2D, l_texture->GetID());

        l_shader->SetUniform("u_Model", l_modelMat);
        Use(l_shader);

        glDrawArrays(GL_TRIANGLES, 0, l_model->GetVertexCount());
                
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Camera::SetPosition(Vector3 _p)
    {
        m_position = _p;
    }

    void Camera::SetLookAtTarget(int _id)
    {
        m_lookAtTarget = _id;

        if(_id == -1)
        {
            m_isLookingAt = false;
        }
        else
        {
            m_isLookingAt = true;
        }
    }

    int Camera::GetLookAtTarget()
    {
        return m_lookAtTarget;
    }

    void Camera::SetLookAtDistance(float _d)
    {
        m_lookAtDistance = _d;
    }

    float Camera::GetLookAtDistance()
    {
        return m_lookAtDistance;
    }

    void Camera::SetProjectionType(int _t)
    {
        if(_t > -1 && _t < 2)
        {
            m_projectionType = (ProjectionType)_t;
        }
    }

    Camera::ProjectionType Camera::GetProjectionType()
    {
        return m_projectionType;
    }
}