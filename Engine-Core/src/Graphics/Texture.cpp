#include <pch.h>
#include "Texture.h"


namespace TGE 
{
	Texture::Texture(const std::string_view& FileLocation, const TextureData& textureData, const Format& format)
		: m_Texture(0), m_TextureData(textureData), m_InternalFormat(GL_RGBA8), m_FileFormat(0)
		
	{
		switch (format)
		{
		case Format::RGB:
			m_FileFormat = GL_RGB;
			m_DesiredChannels = 3;
			break;
		case Format::RGBA:
			m_FileFormat = GL_RGBA;
			m_DesiredChannels = 4;
			break;
		default:
			break;
		}

		switch (m_TextureData.Type)
		{
		case TGE::TextureType::Texture2D:
			m_TextureType = GL_TEXTURE_2D;
			CreateTexture(GL_TEXTURE_2D, FileLocation);
			break;
		default:
			break;
		}
	}

	Texture::Texture(float* Data, const TextureData& textureData)
		: m_Texture(0), m_TextureData(textureData), m_InternalFormat(0), m_FileFormat(GL_RGBA)
	{

		switch (m_TextureData.InternalFormat)
		{
		case TextureFormat::RGBA16F: m_InternalFormat = GL_RGBA16F; break;
		case TextureFormat::RGBA32F: m_InternalFormat = GL_RGBA32F; break;
		default: LOGWARNING("Try using uint32_t instead of floats!"); break;
		}

		switch (m_TextureData.Type)
		{
		case TGE::TextureType::Texture2D: 
			m_TextureType = GL_TEXTURE_2D;
			CreateTexture(GL_TEXTURE_2D, Data);
			break;
		default:
			break;
		}
	}

	Texture::Texture(uint32_t* Data, const TextureData& textureData)
		: m_Texture(0), m_TextureData(textureData), m_InternalFormat(0), m_FileFormat(GL_RGBA), m_TextureType(0)
	{
		switch (m_TextureData.InternalFormat)
		{
		case TextureFormat::RGBA8: m_InternalFormat = GL_RGBA8; break;
		default:
			LOGWARNING("texture format not supported, try using float instead of uint32_t");
			break;
		}

		switch (m_TextureData.Type)
		{
		case TGE::TextureType::Texture2D:
			m_TextureType = GL_TEXTURE_2D;
			CreateTexture(GL_TEXTURE_2D, Data);
			break;
		case TGE::TextureType::Texture2DMultisample:
			m_TextureType = GL_TEXTURE_2D_MULTISAMPLE;
			CreateTexture(GL_TEXTURE_2D_MULTISAMPLE, Data);
			break;
		default:
			break;
		}
	}

	Texture::~Texture()
	{
		glDeleteTextures(1, &m_Texture);
	}

	std::unique_ptr<Texture> Texture::Generate(const std::string_view& FileLocation, const TextureData& textureData, const Format& format)
	{
		return std::make_unique<Texture>(FileLocation, textureData, format);
	}

	std::unique_ptr<Texture> Texture::Generate(float* Data, const TextureData& textureData)
	{
		return std::make_unique<Texture>(Data, textureData);
	}

	std::unique_ptr<Texture> Texture::Generate(uint32_t* Data, const TextureData& textureData)
	{
		return std::make_unique<Texture>(Data, textureData);
	}

	std::shared_ptr<Texture> Texture::GenerateShared(const std::string_view& FileLocation, const TextureData& textureData, const Format& format)
	{
		return std::make_shared<Texture>(FileLocation, textureData, format);
	}

	std::shared_ptr<Texture> Texture::GenerateShared(float* Data, const TextureData& textureData)
	{
		return std::make_shared<Texture>(Data, textureData);
	}

	std::shared_ptr<Texture> Texture::GenerateShared(uint32_t* Data, const TextureData& textureData)
	{
		return std::make_shared<Texture>(Data, textureData);
	}

	void Texture::ResizeImage(float* Data, int Width, int Height)
	{
		m_TextureData.Width = Width;
		m_TextureData.Height = Height;

		glDeleteTextures(1, &m_Texture);

		CreateTexture(m_TextureType, Data);
	}

	void Texture::ResizeImage(uint32_t* Data, int Width, int Height)
	{
		m_TextureData.Width = Width;
		m_TextureData.Height = Height;

		glDeleteTextures(1, &m_Texture);

		CreateTexture(m_TextureType, Data);
	}

	void Texture::SetData(float* Data)
	{
		glBindTexture(m_TextureType, m_Texture);
		glTexSubImage2D(m_TextureType, m_TextureData.MipmapLevels, 0, 0, m_TextureData.Width, m_TextureData.Height,
			m_FileFormat, GL_FLOAT, Data);
	}

	void Texture::SetData(uint32_t* Data)
	{
		glBindTexture(m_TextureType, m_Texture);
		glTexSubImage2D(m_TextureType, m_TextureData.MipmapLevels, 0, 0, m_TextureData.Width, m_TextureData.Height,
			m_FileFormat, GL_UNSIGNED_BYTE, Data);
	}

	void Texture::SetData(const std::string_view& NewImageLocation)
	{
		int Channels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* image = nullptr;

		uint32_t TempWidth = m_TextureData.Width;
		uint32_t TempHeight = m_TextureData.Height;

		image = stbi_load(NewImageLocation.data(), &m_TextureData.Width, &m_TextureData.Height, &Channels, m_DesiredChannels);
		
		const char* Failure = stbi_failure_reason();
		if (Failure)
		{
			LOGERROR(std::string(Failure));
			return;
		}

		if (m_TextureData.Width != TempWidth || m_TextureData.Height != TempHeight)
		{
			glDeleteTextures(1, &m_Texture);

			glBindTexture(m_TextureType, m_Texture);
			glCreateTextures(m_TextureType,1, &m_Texture);

			for (auto& kv : m_TextureData.TextureParamaters)
				glTextureParameteri(m_Texture, kv.first, kv.second);

			UploadTextureImage(m_TextureType, image);
		}
		else
		{
			glBindTexture(m_TextureType, m_Texture);
			glTexSubImage2D(m_TextureType, m_TextureData.MipmapLevels, 0, 0, m_TextureData.Width, m_TextureData.Height,
				m_FileFormat, GL_UNSIGNED_BYTE, image);
		}

		if (image)
			stbi_image_free(image);
	}

	void Texture::CreateTexture(GLenum TextureType, const std::string_view& FileLocation)
	{
		glCreateTextures(TextureType, 1, &m_Texture);
		glBindTexture(TextureType, m_Texture);

		for (auto& kv : m_TextureData.TextureParamaters)
			glTextureParameteri(m_Texture, kv.first, kv.second);


		int Channels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* image = nullptr;

		image = stbi_load(FileLocation.data(), &m_TextureData.Width, &m_TextureData.Height, &Channels, m_DesiredChannels);
		const char* Failure = stbi_failure_reason();

		if (Failure)
		{
			LOGERROR(std::string(Failure));
			return;
		}
		
		UploadTextureImage(TextureType, image);

		if (image)
			stbi_image_free(image);

	}

	void Texture::CreateTexture(GLenum TextureType, float* Data)
	{
		glCreateTextures(TextureType, 1, &m_Texture);
		glBindTexture(TextureType, m_Texture);

		for (auto& kv : m_TextureData.TextureParamaters)
			glTextureParameteri(m_Texture, kv.first, kv.second);
		
		UploadTexture(TextureType, Data);

	}

	void Texture::CreateTexture(GLenum TextureType, uint32_t* Data)
	{
		glCreateTextures(TextureType, 1, &m_Texture);
		glBindTexture(TextureType, m_Texture);

		for (auto& kv : m_TextureData.TextureParamaters)
			glTextureParameteri(m_Texture, kv.first, kv.second);

		UploadTexture(TextureType, Data);
	}

	void Texture::UploadTextureImage(GLenum TextureType, unsigned char* Data)
	{
		switch (TextureType)
		{
		case GL_TEXTURE_2D:
			glTextureStorage2D(m_Texture, 1, m_InternalFormat, m_TextureData.Width, m_TextureData.Height);
			glTexSubImage2D(GL_TEXTURE_2D, m_TextureData.MipmapLevels, 0, 0, m_TextureData.Width, m_TextureData.Height, m_FileFormat,
				GL_UNSIGNED_BYTE, Data);
			glGenerateTextureMipmap(m_Texture);
			break;
		default:
			break;
		}
	}

	void Texture::UploadTexture(GLenum TextureType, float* Data)
	{
		switch (TextureType)
		{
		case GL_TEXTURE_2D:
			glTextureStorage2D(m_Texture, 1, m_InternalFormat, m_TextureData.Width, m_TextureData.Height);
			glTexSubImage2D(GL_TEXTURE_2D, m_TextureData.MipmapLevels, 0, 0, m_TextureData.Width, m_TextureData.Height, m_FileFormat,
				GL_FLOAT, Data);
			glGenerateTextureMipmap(m_Texture);
			break;
		default:
			break;
		}
	}

	void Texture::UploadTexture(GLenum TextureType, uint32_t* Data)
	{
		switch (TextureType)
		{
		case GL_TEXTURE_2D:
			glTextureStorage2D(m_Texture, 1, m_InternalFormat, m_TextureData.Width, m_TextureData.Height);
			glTexSubImage2D(GL_TEXTURE_2D, m_TextureData.MipmapLevels, 0, 0, m_TextureData.Width, m_TextureData.Height, m_FileFormat,
				GL_UNSIGNED_BYTE, Data);
			glGenerateTextureMipmap(m_Texture);
			break;
		case GL_TEXTURE_2D_MULTISAMPLE:
			glTextureStorage2DMultisample(m_Texture, m_TextureData.NumberOfSamples, m_InternalFormat, m_TextureData.Width, m_TextureData.Height, GL_TRUE);
			glTexSubImage2D(GL_TEXTURE_2D_MULTISAMPLE, m_TextureData.MipmapLevels, 0, 0, m_TextureData.Width, m_TextureData.Height, m_FileFormat,
				GL_UNSIGNED_BYTE, Data);
			break;
		default:
			break;
		}
	}
	
}
