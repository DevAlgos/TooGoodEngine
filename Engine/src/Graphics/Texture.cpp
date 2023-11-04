#include <pch.h>
#include "Texture.h"


namespace Graphics 
{
	Texture::Texture(const TextureData& textureData)
		: m_Texture(0), m_TextureData(textureData)
	{

		switch (m_TextureData.Type)
		{
		case TextureType::Texture1D:
			CreateTexture(GL_TEXTURE_1D);
			break;
		case TextureType::Texture2D:
			CreateTexture(GL_TEXTURE_2D);
			break;
		case TextureType::Texture3D:
			LOGERROR("Engine hasn't branched to 3D yet!");
			break;
		default:
			LOGERROR("That texture type is currently not supported!");
			break;
		}

	}

	void Texture::UploadTextureImmutable(GLenum TextureType)
	{
		switch (TextureType)
		{
		case GL_TEXTURE_1D:
			if (m_TextureData.UseData8)
			{
				glTextureStorage1D(m_Texture, m_TextureData.MipmapLevels, m_TextureData.InternalFormat, m_TextureData.Width);
				glTexSubImage1D(GL_TEXTURE_1D, m_TextureData.MipmapLevels, 0, m_TextureData.Width, m_TextureData.InternalFormat,
					GL_UNSIGNED_BYTE, m_TextureData.Data8);
				glGenerateTextureMipmap(m_Texture);
			}
			else if (m_TextureData.UseData16)
			{

				glTextureStorage1D(m_Texture, m_TextureData.MipmapLevels, m_TextureData.InternalFormat, m_TextureData.Width);
				glTexSubImage1D(GL_TEXTURE_1D, m_TextureData.MipmapLevels, 0, m_TextureData.Width, m_TextureData.InternalFormat,
					GL_UNSIGNED_SHORT, m_TextureData.Data16);
				glGenerateTextureMipmap(m_Texture);
			}
			else if (m_TextureData.UseData32)
			{

				glTextureStorage1D(m_Texture, m_TextureData.MipmapLevels, m_TextureData.InternalFormat, m_TextureData.Width);
				glTexSubImage1D(GL_TEXTURE_1D, m_TextureData.MipmapLevels, 0, m_TextureData.Width, m_TextureData.InternalFormat,
					GL_UNSIGNED_INT, m_TextureData.Data32);
				glGenerateTextureMipmap(m_Texture);
			}
			else
			{
				LOGERROR("Data is null");
			}
			break;
		case GL_TEXTURE_2D:
			if (m_TextureData.UseData8)
			{
				glTextureStorage2D(m_Texture, m_TextureData.MipmapLevels, m_TextureData.InternalFormat, m_TextureData.Width, m_TextureData.Height);
				glTexSubImage2D(GL_TEXTURE_2D, m_TextureData.MipmapLevels, 0, 0, m_TextureData.Width, m_TextureData.Height, m_TextureData.InternalFormat,
					GL_UNSIGNED_BYTE, m_TextureData.Data8);
				glGenerateTextureMipmap(m_Texture);
			}
			else if (m_TextureData.UseData16)
			{

				glTextureStorage2D(m_Texture, m_TextureData.MipmapLevels, m_TextureData.InternalFormat, m_TextureData.Width, m_TextureData.Height);
				glTexSubImage2D(GL_TEXTURE_2D, m_TextureData.MipmapLevels, 0, 0, m_TextureData.Width, m_TextureData.Height, m_TextureData.InternalFormat,
					GL_UNSIGNED_BYTE, m_TextureData.Data16);
				glGenerateTextureMipmap(m_Texture);
			}
			else if (m_TextureData.UseData32)
			{

				glTextureStorage2D(m_Texture, m_TextureData.MipmapLevels, m_TextureData.InternalFormat, m_TextureData.Width, m_TextureData.Height);
				glTexSubImage2D(GL_TEXTURE_2D, m_TextureData.MipmapLevels, 0, 0, m_TextureData.Width, m_TextureData.Height, m_TextureData.InternalFormat,
					GL_UNSIGNED_BYTE, m_TextureData.Data32);
				glGenerateTextureMipmap(m_Texture);
			}
			else
			{
				LOGWARNING("Data is null");
				glTextureStorage2D(m_Texture, m_TextureData.MipmapLevels, m_TextureData.InternalFormat, m_TextureData.Width, m_TextureData.Height);
				glTexSubImage2D(GL_TEXTURE_2D, m_TextureData.MipmapLevels, 0, 0, m_TextureData.Width, m_TextureData.Height, m_TextureData.InternalFormat,
					GL_UNSIGNED_BYTE, nullptr);
				glGenerateTextureMipmap(m_Texture);
			}
			break;
		default:
			break;
		}
	}
	void Texture::UploadTextureImmutable(GLenum TextureType, const char* Filename)
	{
		int width, height, Channels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* image = nullptr;

		image = stbi_load(m_TextureData.FileLocation, &width, &height, &Channels, 0);

		switch (TextureType)
		{
		case GL_TEXTURE_1D:
			glTextureStorage1D(m_Texture, m_TextureData.MipmapLevels, m_TextureData.InternalFormat, width);
			glTexSubImage1D(GL_TEXTURE_1D, m_TextureData.MipmapLevels, 0, width, m_TextureData.InternalFormat,
				GL_UNSIGNED_BYTE, image);
			glGenerateTextureMipmap(m_Texture);
			break;
		case GL_TEXTURE_2D:
			glTextureStorage2D(m_Texture, m_TextureData.MipmapLevels, m_TextureData.InternalFormat, width, height);
			glTexSubImage2D(GL_TEXTURE_2D, m_TextureData.MipmapLevels, 0, 0, width, height, m_TextureData.InternalFormat,
				GL_UNSIGNED_BYTE, image);
			glGenerateTextureMipmap(m_Texture);
			break;
		default:
			break;
		}


		if (image)
			stbi_image_free(image);
	}

	void Texture::UploadTexture(GLenum TextureType)
	{
		switch (TextureType)
		{
		case GL_TEXTURE_1D:
			if (m_TextureData.UseData8)
			{
				glTexImage1D(GL_TEXTURE_1D, m_TextureData.MipmapLevels, m_TextureData.InternalFormat, m_TextureData.Width, 0, m_TextureData.InternalFormat,
					GL_UNSIGNED_BYTE, m_TextureData.Data8);
				glGenerateTextureMipmap(m_Texture);
			}
			else if (m_TextureData.UseData16)
			{
				glTexImage1D(GL_TEXTURE_1D, m_TextureData.MipmapLevels, m_TextureData.InternalFormat, m_TextureData.Width, 0, m_TextureData.InternalFormat,
					GL_UNSIGNED_SHORT, m_TextureData.Data16);
				glGenerateTextureMipmap(m_Texture);
			} 
			else if (m_TextureData.UseData32)
			{
				glTexImage1D(GL_TEXTURE_1D, m_TextureData.MipmapLevels, m_TextureData.InternalFormat, m_TextureData.Width, 0, m_TextureData.InternalFormat,
					GL_UNSIGNED_INT, m_TextureData.Data32);
				glGenerateTextureMipmap(m_Texture);
			}
		
			else
			{
				LOGWARNING("Data is null!");
				glTexImage1D(GL_TEXTURE_1D, m_TextureData.MipmapLevels, m_TextureData.InternalFormat, m_TextureData.Width, 0, m_TextureData.InternalFormat,
					GL_UNSIGNED_BYTE, nullptr);
				glGenerateTextureMipmap(m_Texture);
			}
			break;
		case GL_TEXTURE_2D:
			if (m_TextureData.UseData8)
			{
				glTexImage2D(GL_TEXTURE_2D, m_TextureData.MipmapLevels, m_TextureData.InternalFormat, m_TextureData.Width, m_TextureData.Height, 0,
					m_TextureData.InternalFormat, GL_UNSIGNED_BYTE, m_TextureData.Data8);
				glGenerateTextureMipmap(m_Texture);
			} 
			else if (m_TextureData.UseData16)
			{
				glTexImage2D(GL_TEXTURE_2D, m_TextureData.MipmapLevels, m_TextureData.InternalFormat, m_TextureData.Width, m_TextureData.Height, 0,
					m_TextureData.InternalFormat, GL_UNSIGNED_SHORT, m_TextureData.Data16);
				glGenerateTextureMipmap(m_Texture);
			}
			else if (m_TextureData.UseData32)
			{
				glTexImage2D(GL_TEXTURE_2D, m_TextureData.MipmapLevels, m_TextureData.InternalFormat, m_TextureData.Width, m_TextureData.Height, 0,
					m_TextureData.InternalFormat, GL_UNSIGNED_INT, m_TextureData.Data32);
				glGenerateTextureMipmap(m_Texture);
			} 
			else
			{
				LOGWARNING("Data is nullptr");
				glTexImage2D(GL_TEXTURE_2D, m_TextureData.MipmapLevels, m_TextureData.InternalFormat, m_TextureData.Width, m_TextureData.Height, 0,
					m_TextureData.InternalFormat, GL_UNSIGNED_BYTE, nullptr);
				glGenerateTextureMipmap(m_Texture);
			}
			break;
		default:
			break;
		}
	}
	void Texture::UploadTexture(GLenum TextureType, const char* Filename)
	{
		int width, height, Channels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* image = nullptr;

		image = stbi_load(m_TextureData.FileLocation, &width, &height, &Channels, 0);

		switch (TextureType)
		{
		case GL_TEXTURE_1D:
			glTexImage1D(GL_TEXTURE_1D, m_TextureData.MipmapLevels, m_TextureData.InternalFormat, width, 0, m_TextureData.InternalFormat,
				GL_UNSIGNED_BYTE, image);
			glGenerateTextureMipmap(m_Texture);
			break;
		case GL_TEXTURE_2D:
			glTexImage2D(GL_TEXTURE_2D, m_TextureData.MipmapLevels, m_TextureData.InternalFormat, width, height, 0,
				m_TextureData.InternalFormat, GL_UNSIGNED_BYTE, image);
			glGenerateTextureMipmap(m_Texture);
			break;
		default:
			break;
		}

		if (image)
			stbi_image_free(image);
	}

	void Texture::CreateTexture(GLenum TextureType)
	{
		glCreateTextures(TextureType, 1, &m_Texture);
		glBindTexture(TextureType, m_Texture);

		for (auto& kv : m_TextureData.TextureParamaters)
			glTextureParameteri(m_Texture, kv.first, kv.second);

		if (m_TextureData.FileLocation)
		{
			if (m_TextureData.Immutable)
				UploadTextureImmutable(TextureType, m_TextureData.FileLocation);
			else
				UploadTexture(TextureType, m_TextureData.FileLocation);
		}
		else
		{
			if (m_TextureData.Immutable)
				UploadTextureImmutable(TextureType);
			else
				UploadTexture(TextureType);
		}


	}

	Texture::~Texture()
	{
		glDeleteTextures(1, &m_Texture);
	}
}
