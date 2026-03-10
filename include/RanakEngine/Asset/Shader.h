#ifndef SHADER_H
#define SHADER_H

#include "RanakEngine/Asset/AssetFile.h"

#include "RanakEngine/Math/Vector2.h"
#include "RanakEngine/Math/Vector3.h"
#include "RanakEngine/Math/Vector4.h"

#include "GL/glew.h"
#include "GLM/ext.hpp"

#include <string>

namespace RanakEngine::Asset
{
    /**
     * @enum ShaderType
     * @brief Enumeration of shader program types.
     */
    enum ShaderType
    {
        Compute = 0,    ///< Compute shader
        FragVert = 1    ///< Fragment and vertex shader pair
    };

    /**
     * @class Shader
     * @brief Asset class for loading and managing OpenGL shader programs.
     * 
     * Compiles and links shader source files into OpenGL shader programs.
     * Supports both compute shaders and traditional vertex/fragment pairs.
     * 
     * @see Asset
     */
    class Shader : public AssetFile
    {
        private:
        GLuint m_ID;                ///< OpenGL shader program ID
        ShaderType m_shaderType;    ///< The type of shader program

        public:
        /**
         * @brief Constructs a shader from a file.
         * 
         * @param _computePath Path to the compute shader source file.
         */
        Shader(std::string _paths);

        /**
         * @brief Destructs the Shader and releases OpenGL resources.
         */
        ~Shader();

        /**
         * @brief Activates this shader for rendering.
         */
        void Use();

        /**
         * @brief Sets a mat4 uniform value.
         * 
         * @param _uniformName Name of the uniform variable.
         * @param _value The matrix value to set.
         */
        void SetUniform(const std::string& _uniformName, glm::mat4 _value);

        /**
         * @brief Sets a mat3 uniform value.
         * 
         * @param _uniformName Name of the uniform variable.
         * @param _value The matrix value to set.
         */
        void SetUniform(const std::string& _uniformName, glm::mat3 _value);

        /**
         * @brief Sets a vec4 uniform value.
         * 
         * @param _uniformName Name of the uniform variable.
         * @param _value The vector value to set.
         */
        void SetUniform(const std::string& _uniformName, Vector4 _value);

        /**
         * @brief Sets a vec3 uniform value.
         * 
         * @param _uniformName Name of the uniform variable.
         * @param _value The vector value to set.
         */
        void SetUniform(const std::string& _uniformName, Vector3 _value);

        /**
         * @brief Sets a vec2 uniform value.
         * 
         * @param _uniformName Name of the uniform variable.
         * @param _value The vector value to set.
         */
        void SetUniform(const std::string& _uniformName, Vector2 _value);

        /**
         * @brief Sets a float uniform value.
         * 
         * @param _uniformName Name of the uniform variable.
         * @param _value The float value to set.
         */
        void SetUniform(const std::string& _uniformName, float _value);

        /**
         * @brief Gets the OpenGL shader program ID.
         * 
         * @return GLuint& Reference to the program ID.
         */
        GLuint& GetID();

        /**
         * @brief Gets the shader type.
         * 
         * @return ShaderType& Reference to the shader type.
         */
        ShaderType& GetShaderType();
    };
};

#endif