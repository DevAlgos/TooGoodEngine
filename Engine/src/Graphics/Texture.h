#pragma once

#include <glad/glad.h>
#include <iostream>
#include <unordered_map>

namespace TGE {

	enum class TextureType 
	{   // Will add more when needed, however Texture2D is all thats required for now
		Texture2D = 0, 
		Texture3D, 
		Texture2DCompute,
	    TextureCubeMap, TextureRectangle, 
		Texture2DMultiSampling, Texture2DArrayTexture,
		TextureBuffer, Texture2DMultiSampleArray,
		TextureSparse
	};

	enum class TextureFormat
	{
		RGBA16F = 0, RGBA32F
	};

	enum class Format
	{
		RGB = 0, RGBA
	};
	
	struct TextureData
	{
		TextureFormat InternalFormat = TextureFormat::RGBA16F; //This is irrelevant for sampling from a file as automatically will be GL_RGBA8
		TextureType Type = TextureType::Texture2D;
		int Width, Height = 0; //leave empty if specifiying a texture from file as these will be sampled from the image
		int MipmapLevels = 0;

		std::unordered_map<GLenum, GLenum> TextureParamaters =
		{
			{GL_TEXTURE_MIN_FILTER, GL_NEAREST},
			{GL_TEXTURE_MAG_FILTER, GL_NEAREST}
		};

	};

	class Texture
	{
	public:
		Texture(const std::string_view& FileLocation, const TextureData& textureData, const Format& format);
		Texture(float* Data, const TextureData& textureData);
		~Texture();

		void ResizeImage(float* Data, int Width, int Height);

		void SetData(float* Data);
		void SetData(const std::string_view& NewImageLocation);

		inline void Bind(int Unit) { glBindTextureUnit(Unit, m_Texture); }
		inline void BindImage(int Unit) { glBindImageTexture(Unit, m_Texture, m_TextureData.MipmapLevels, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA); };
		inline const uint32_t Get() const { return m_Texture; }

	private:
		void CreateTexture(GLenum TextureType, const std::string_view& FileLocation);
		void CreateTexture(GLenum TextureType, float* Data);

		void UploadTextureImage(GLenum TextureType, unsigned char* Data);
		void UploadTexture(GLenum TextureType, float* Data);

	private:
		uint32_t m_Texture;
		TextureData m_TextureData;
		
		GLenum m_InternalFormat;
		GLenum m_FileFormat;
	};
}