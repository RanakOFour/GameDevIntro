#include "RanakEngine/Core/Camera.h"
#include "RanakEngine/Assets.h"
#include "RanakEngine/Math.h"

#include "sol/sol.hpp"
#include "GL/glew.h"
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
    , m_projectionType(ProjectionType::Orthographic)
    {
        //m_projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
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

    glm::mat4 CalculateModelMatrix(Vector3 _pos, Vector3 _euler)
    {
        glm::vec3 l_glPos = glm::vec3(_pos.x, _pos.y, _pos.z);
        glm::quat l_glQuat = (glm::quat)Quaternion(glm::radians(_euler.x), glm::radians(_euler.y), glm::radians(_euler.z));

        glm::mat4 translation = glm::translate(glm::mat4(1.0f), l_glPos);
        glm::mat4 rotation = glm::mat4(l_glQuat);
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
        
        return translation * rotation * scale;
    }

    void Camera::Draw(sol::table _transform, sol::table _drawable)
    {
        // Get model, texture, and shader from drawable, loading them if they haven't been already
        auto l_assetManager = Asset::Manager::Instance().lock();

        sol::optional<std::weak_ptr<Asset::Model>> l_modelPtr = _drawable.raw_get<sol::optional<std::weak_ptr<Asset::Model>>>("model");

        if(!l_modelPtr.has_value())
        {
            // Try to load model from path in drawable, otherwise load default model
            std::string l_modelPath = _drawable.raw_get<std::string>("modelPath");
            if(l_modelPath == "")
            {
                l_modelPath = "./resources/models/FlatTexture.obj";
            }

            _drawable.raw_set("model", l_assetManager->Load<Asset::Model>(l_modelPath));
            l_modelPtr = _drawable.raw_get<std::weak_ptr<Asset::Model>>("model");
        }

        auto l_model = l_modelPtr.value().lock();

        sol::optional<std::weak_ptr<Asset::Texture>> l_texturePtr = _drawable.raw_get<sol::optional<std::weak_ptr<Asset::Texture>>>("texture");

        if(!l_texturePtr.has_value())
        {
            // Try to load texture from path in drawable, otherwise load default texture
            std::string l_texturePath = _drawable.raw_get<std::string>("texturePath");
            if(l_texturePath == "")
            {
                l_texturePath = "./resources/textures/triangle.png";
            }

            _drawable.raw_set("texture", l_assetManager->Load<Asset::Texture>(l_texturePath));
            l_texturePtr = _drawable.raw_get<std::weak_ptr<Asset::Texture>>("texture");
        }

        auto l_texture = l_texturePtr.value().lock();

        sol::optional<std::weak_ptr<Asset::Shader>> l_shaderPtr = _drawable.raw_get<sol::optional<std::weak_ptr<Asset::Shader>>>("shader");

        if(!l_shaderPtr.has_value())
        {
            // Try to load shader from path in drawable, otherwise load default shader
            std::string l_shaderPath = _drawable.raw_get<std::string>("shaderPath");
            if(l_shaderPath == "")
            {
                l_shaderPath = "./resources/shaders/default/frag.fs;./resources/shaders/default/vert.vs";
            }

            _drawable.raw_set("shader", l_assetManager->Load<Asset::Shader>(l_shaderPath));
            l_shaderPtr = _drawable.raw_get<std::weak_ptr<Asset::Shader>>("shader");
        }

        auto l_shader = l_shaderPtr.value().lock();
        
        l_shader->Use();
        
        glBindVertexArray(l_model->GetVAO());

        glBindTexture(GL_TEXTURE_2D, l_texture->GetID());

        glm::mat4 l_modelMat = CalculateModelMatrix(_transform.raw_get<Vector3>("position"), _transform.raw_get<Vector3>("rotation"));

        l_shader->SetUniform("u_Model", l_modelMat);
        
        l_modelMat = CalculateModelMatrix(m_position, Vector3(0.0f, m_rotation, 0.0f));

        l_shader->SetUniform("u_View", glm::inverse(l_modelMat));
        l_shader->SetUniform("u_Projection", m_projection);

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