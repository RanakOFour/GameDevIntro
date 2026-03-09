#ifndef TEXTURE_H
#define TEXTURE_H

#include "RanakEngine/Asset/AssetFile.h"

#include "GL/glew.h"
#include "GLM/ext.hpp"

#include <string>
#include <vector>

namespace RanakEngine::Asset
{
    /**
     * @class Texture
     * @brief Asset class for loading and managing OpenGL textures.
     * 
     * Loads image data from disk and creates OpenGL texture objects.
     * Supports common image formats through stb_image.
     * 
     * @see Asset
     */
    class Texture : public AssetFile
    {
    protected:
        bool m_dirty;       ///< Whether the texture needs to be reuploaded
        glm::ivec2 m_size;  ///< Texture dimensions in pixels
        GLuint m_id;        ///< OpenGL texture ID

    public:
        /**
         * @brief Constructs a Texture from an image file.
         * 
         * @param _path Path to the image file to load.
         */
        Texture(std::string _path);

        /**
         * @brief Copy assignment operator.
         * 
         * @param _assign The texture to copy from.
         * 
         * @return Texture& Reference to this texture.
         */
        Texture& operator=(const Texture& _assign);

        /**
         * @brief Destructs the Texture and releases OpenGL resources.
         */
        ~Texture();

        /**
         * @brief Sets the texture dimensions.
         * 
         * @param _size The new size in pixels.
         */
        void Size(glm::ivec2 _size);

        /**
         * @brief Gets the texture dimensions.
         * 
         * @return const glm::ivec2 The size in pixels.
         */
        const glm::ivec2 Size();

        /**
         * @brief Loads texture data from a file.
         * 
         * @param _path Path to the image file to load.
         */
        virtual void Load(const std::string& _path);

        /**
         * @brief Gets the OpenGL texture ID.
         * 
         * @return GLuint The texture ID.
         */
        GLuint GetID();
    };
};

#endif