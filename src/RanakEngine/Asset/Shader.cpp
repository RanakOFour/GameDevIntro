#include "RanakEngine/Asset/Shader.h"
#include "RanakEngine/Log.h"

#include <vector>
#include <fstream>
#include <sstream>

namespace RanakEngine::Asset
{
    Shader::Shader(std::string _path)
    : AssetFile(_path, AssetType::SHADER)
    , m_ID(0)
    {
        int l_splitPos = _path.find_first_of(';');

        if(l_splitPos > 0)
        {
            // Multiple paths in shader
            m_shaderType = ShaderType::FragVert;

            std::string l_vertPath = _path.substr(0, l_splitPos);
            std::string l_fragPath = _path.substr(l_splitPos + 1, _path.size() - l_splitPos);

            int l_fileExtensionIndex = l_vertPath.find_last_of('.') + 1;
            std::string l_vertFileType = l_vertPath.substr(l_fileExtensionIndex, l_vertPath.size() - l_fileExtensionIndex);

            // Swap paths around if frag path is first
            if(l_vertFileType != "vs" && l_vertFileType != "vert")
            {
                std::string l_temp = l_vertPath;
                l_vertPath = l_fragPath;
                l_fragPath = l_temp;
            }

            Log::Message("Attempting to create fragvert shader with paths:\n" + l_vertPath + "\n" + l_fragPath + "\npaths.");

            std::ifstream l_stream;
            l_stream.open(l_vertPath);

            std::stringstream l_fileContents;
            l_fileContents << l_stream.rdbuf();
            l_stream.close();

            std::string l_shaderCode = l_fileContents.str();
            char* l_code = (char*)l_shaderCode.c_str();

            std::string l_debug = "Vertex shader code: " + l_shaderCode;
            Log::Debug(l_debug);
            
            GLuint l_vertexShader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(l_vertexShader, 1, &l_code, NULL);

            glCompileShader(l_vertexShader);

            GLint success = 0;
            glGetShaderiv(l_vertexShader, GL_COMPILE_STATUS, &success);
            if (success != GL_TRUE)
            {
                char L_errorLog[1024];
                glGetShaderInfoLog(l_vertexShader, 1024, nullptr, &L_errorLog[0]);

                GLenum l_errorEnum = glGetError();
                // GL_NO_ERROR is 0
                
                if(l_errorEnum != 0)
                {
                    std::string l_errorString = "Could not compile vertex shader: Errornum: " + std::to_string(l_errorEnum) + ". " + std::string(L_errorLog);
                    Log::Error(l_errorString);

                    glDeleteVertexShaderEXT(l_vertexShader);
                }
            }
            
            l_stream.open(l_fragPath);

            l_fileContents = std::stringstream();
            l_fileContents << l_stream.rdbuf();
            l_stream.close();
            
            l_shaderCode = l_fileContents.str();
            l_code = (char*)l_shaderCode.c_str();

            l_debug = "Fragment shader code: " + l_shaderCode;
            Log::Debug(l_debug);

            // Create a new fragment shader
            GLuint l_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            // Upload shader code to GPU
            glShaderSource(l_fragmentShader, 1, &l_code, NULL);
            // Compiler fragment shader
            glCompileShader(l_fragmentShader);
            // Get success val for shader compilation
            glGetShaderiv(l_fragmentShader, GL_COMPILE_STATUS, &success);

            if (success != GL_TRUE)
            {
                char L_errorLog[1024];
                glGetShaderInfoLog(l_fragmentShader, 1024, nullptr, &L_errorLog[0]);

                GLenum l_errorEnum = glGetError();
                // GL_NO_ERROR is 0
                if(l_errorEnum != 0)
                {
                    std::string l_errorString = "Could not compile fragment shader: Errornum: " + std::to_string(l_errorEnum) + ". " + std::string(L_errorLog);
                    Log::Error(l_errorString);

                    glDeleteFragmentShaderATI(l_fragmentShader);
                }
            }

            printf("Vertex frag created\n");

            // Create new shader program and attach shader objects
            m_ID = glCreateProgram();
            glAttachShader(m_ID, l_vertexShader);
            glAttachShader(m_ID, l_fragmentShader);

            // Associated values in VBOs to variables in the shader code
            glBindAttribLocation(m_ID, 0, "a_Position");
            glBindAttribLocation(m_ID, 1, "a_PixelColor");

            // Perform the link and check for failure
            glLinkProgram(m_ID);
            glGetProgramiv(m_ID, GL_LINK_STATUS, &success);

            if (success != GL_TRUE)
            {
                GLenum l_errorEnum = glGetError();
                // GL_NO_ERROR is 0
                if(l_errorEnum != 0)
                {
                    Log::Error("Failed to compile shader program. Error code: " + std::to_string(l_errorEnum));
                }
            }

            //printf("Shader program created\n");

            // Detach and destroy the shader objects. These are no longer needed
            // because we now have a complete shader program.
            glDetachShader(m_ID, l_vertexShader);
            glDeleteShader(l_vertexShader);
            glDetachShader(m_ID, l_fragmentShader);
            glDeleteShader(l_fragmentShader);
        }
        else
        {
            // Only 1 shader path == compute
            m_shaderType = ShaderType::Compute;
            std::ifstream shaderFile;
            shaderFile.open(_path);

            std::stringstream fileStream;
            fileStream << shaderFile.rdbuf();
            shaderFile.close();

            std::string shaderCode = fileStream.str();

            const char* l_code = shaderCode.c_str();

            GLuint compute = glCreateShader(GL_COMPUTE_SHADER);
            glShaderSource(compute, 1, &l_code, NULL);
            glCompileShader(compute);

            GLint success;
            GLchar infoLog[1000];
            glGetShaderiv(compute, GL_COMPILE_STATUS, &success);
            if(!success)
            {
                glGetShaderInfoLog(compute, 1024, NULL, infoLog);
                Log::Error("SHADER_COMPILATION_ERROR of type: Compute\n" + std::string(infoLog) + "\n -- --------------------------------------------------- -- ");
            }

            m_ID = glCreateProgram();
            glAttachShader(m_ID, compute);
            glLinkProgram(m_ID);

            glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
            if(!success)
            {
                glGetProgramInfoLog(m_ID, 1024, NULL, infoLog);
                Log::Error("PROGRAM_LINKING_ERROR of type: Program\n" + std::string(infoLog) + "\n -- --------------------------------------------------- -- ");
            }

            glDeleteShader(compute);
        }
    }

    Shader::~Shader()
    {
        if(m_ID != 0)
        {
            glDeleteProgram(m_ID);
        }
    }

    void Shader::Use()
    {
        glUseProgram(m_ID);
    }

    void Shader::SetUniform(const std::string& _uniformName, glm::mat4 _value)
    {
        glUniformMatrix4fv(glGetUniformLocation(m_ID, _uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(_value));
    }

    void Shader::SetUniform(const std::string& _uniformName, glm::mat3 _value)
    {
        glUniformMatrix3fv(glGetUniformLocation(m_ID, _uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(_value));
    }

    void Shader::SetUniform(const std::string& _uniformName, Vector4 _value)
    {
        glUniform4f(glGetUniformLocation(m_ID, _uniformName.c_str()), _value.x, _value.y, _value.z, _value.w);
    }

    void Shader::SetUniform(const std::string& _uniformName, Vector3 _value)
    {
        glUniform3f(glGetUniformLocation(m_ID, _uniformName.c_str()), _value.x, _value.y, _value.z);
    }

    void Shader::SetUniform(const std::string& _uniformName, Vector2 _value)
    {
        glUniform2f(glGetUniformLocation(m_ID, _uniformName.c_str()), _value.x, _value.y);
    }

    void Shader::SetUniform(const std::string& _uniformName, float _value)
    {
        glUniform1f(glGetUniformLocation(m_ID, _uniformName.c_str()), _value);
    }

    GLuint& Shader::GetID()
    {
        return m_ID;
    }

    ShaderType& Shader::GetShaderType()
    {
        return m_shaderType;
    }
}