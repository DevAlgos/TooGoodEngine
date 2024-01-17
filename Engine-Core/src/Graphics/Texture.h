#pragma once

#include <glad/glad.h>
#include <iostream>
#include <unordered_map>

namespace TooGoodEngine {

	enum class TextureType 
	{   // Will add more when needed
		Texture2D = 0, 
		Texture2DMultisample
	};

	enum class TextureFormat
	{
		RGBA16F = 0, RGBA32F, RGBA8, RGBA
	};

	enum class Format
	{
		RGB = 0, RGBA
	};
	
	struct TextureData
	{
		TextureFormat InternalFormat = TextureFormat::RGBA16F; //This is irrelevant for sampling from a file as automatically will be GL_RGBA8
		TextureType Type = TextureType::Texture2D;
		int Width = 0, Height = 0; //leave empty if specifiying a texture from file as these will be sampled from the image
		int MipmapLevels = 1;
		int NumberOfSamples = 4;
		int Level = 0;

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
		Texture(uint32_t* Data, const TextureData& textureData);
		~Texture();

		static std::unique_ptr<Texture> Generate(const std::string_view& FileLocation, const TextureData& textureData, const Format& format);
		static std::unique_ptr<Texture> Generate(float* Data, const TextureData& textureData);
		static std::unique_ptr<Texture> Generate(uint32_t* Data, const TextureData& textureData);

		static std::shared_ptr<Texture> GenerateShared(const std::string_view& FileLocation, const TextureData& textureData, const Format& format);
		static std::shared_ptr<Texture> GenerateShared(float* Data, const TextureData& textureData);
		static std::shared_ptr<Texture> GenerateShared(uint32_t* Data, const TextureData& textureData);
		
		void ResizeImage(float* Data, int Width, int Height);
		void ResizeImage(uint32_t* Data, int Width, int Height);


		void SetData(float* Data);
		void SetData(uint32_t* Data);
		void SetData(const std::string_view& NewImageLocation);

		inline void Bind(int Unit) { glBindTextureUnit(Unit, m_Texture); }
		inline void BindImage(int Unit) { glBindImageTexture(Unit, m_Texture, m_TextureData.Level, GL_FALSE, 0, GL_WRITE_ONLY, m_InternalFormat); };
		inline const uint32_t Get() const { return m_Texture; }

	private:
		void CreateTexture(GLenum TextureType, const std::string_view& FileLocation);
		void CreateTexture(GLenum TextureType, float* Data);
		void CreateTexture(GLenum TextureType, uint32_t* Data);


		void UploadTextureImage(GLenum TextureType, unsigned char* Data);
		void UploadTexture(GLenum TextureType, float* Data);
		void UploadTexture(GLenum TextureType, uint32_t* Data);


	private:
		uint32_t m_Texture;
		TextureData m_TextureData;
		
		GLenum m_InternalFormat;
		GLenum m_FileFormat;
		GLenum m_TextureType;
		int m_DesiredChannels = 0;
	};
}