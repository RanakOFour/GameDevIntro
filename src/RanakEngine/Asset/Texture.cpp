#include "RanakEngine/Asset/Texture.h"

#include "GL/glew.h"
#include "GLM/ext.hpp"
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace RanakEngine::Asset
{
	Texture::Texture(std::string _path)
	: AssetFile(_path, AssetType::TEXTURE)
	, m_dirty(true)
	, m_size(0, 0)
	, m_id(0)
	{
		//stbi_set_flip_vertically_on_load(true);

		unsigned char* l_udata = stbi_load(_path.c_str(), &m_size.x, &m_size.y, NULL, 4);

		if (!l_udata || l_udata == NULL)
		{
			printf("Failed to load texture\n");
			throw std::exception();
		}

		char* l_sdata = (char*)l_udata;

		m_contents.assign(l_sdata, l_sdata + m_size.x * m_size.y * 4);
	}

	Texture::~Texture()
	{
		// delete data in vbo
		glDeleteTextures(1, &m_id);
	}

	GLuint Texture::GetID()
	{
		if (m_dirty)
		{
			if (!m_id)
			{
				glGenTextures(1, &m_id);
			}

			glBindTexture(GL_TEXTURE_2D, m_id);

			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			// Upload the image data to the bound texture unit in the GPU
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_size.x, m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_contents.data());

			// Generate Mipmap so the texture can be mapped correctly
			glGenerateMipmap(GL_TEXTURE_2D);

			// Unbind the texture because we are done operating on it
			glBindTexture(GL_TEXTURE_2D, 0);

			printf("Texture %d cleaned\n", m_id);

			m_dirty = false;
		}

		return m_id;
	}

	void Texture::Size(glm::ivec2 _size)
	{
		m_size = _size;

		// Change size of vbo on gpu
	}

	const glm::ivec2 Texture::Size()
	{
		return m_size;
	}

	void Texture::Load(const std::string& _path)
	{
		if (!m_dirty)
		{
			glDeleteBuffers(1, &m_id);
		}

		unsigned char* data = stbi_load(_path.c_str(), &m_size.x, &m_size.y, NULL, 4);

		if (!data || data == NULL)
		{
			printf("Texture::Load -> Data to load is NULL or invalid!\n");
			throw std::exception();
		}

		for (int i = 0; i < m_size.y; i++)
		{
			for (int j = 0; j < m_size.x; j++)
			{
				int newCol = 0;

				// Iterate once for each channel (4)
				for (int k = 0; k < 4; k++)
				{
					int placesToShift = 32 - (8 * k + 1);

					// Put channel's color bits into the correct place
					newCol += (int)*data << placesToShift;
					++data;
				}

				m_contents.push_back((float)newCol);
			}
		}

		m_dirty = true;
	}
};