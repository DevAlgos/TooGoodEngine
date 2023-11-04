#pragma once

#include <glad/glad.h>
#include <iostream>
#include <unordered_map>

namespace Graphics {


	enum class TextureType { // Will add more when needed
		Texture1D = 0,
		Texture2D, 
		Texture3D, //still not supported as engine is currently 2D only at the moment
		Texture2DCompute,
	    TextureCubeMap, TextureRectangle, //These are not supported yet
		Texture2DMultiSampling, Texture2DArrayTexture,
		TextureBuffer, Texture2DMultiSampleArray,
		TextureSparse
	};
	
	struct TextureData
	{
		union
		{
			bool UseFile;
			bool UseData8;
			bool UseData16;
			bool UseData32;
		};

		union 
		{
			const char* FileLocation = nullptr;
			uint8_t*     Data8;
			uint16_t*    Data16;
			uint32_t*    Data32;
		};
		
		GLenum InternalFormat;
		TextureType Type;
		uint32_t Width, Height; //leave empty if specifiying a texture from file as these will be sampled from the image
		bool Immutable = false;
		int MipmapLevels = 0;

		std::unordered_map<GLenum, GLenum> TextureParamaters;

	};

	class Texture
	{
	public:
		Texture(const TextureData& textureData);
		~Texture();

		inline void Bind(int Unit) { glBindTextureUnit(Unit, m_Texture); }
		inline void BindImage(int Unit) { glBindImageTexture(Unit, m_Texture, m_TextureData.MipmapLevels, GL_FALSE, 0, GL_WRITE_ONLY, m_TextureData.InternalFormat); };
		inline const uint32_t Get() const { return m_Texture; }

	private:
		void CreateTexture(GLenum TextureType);

		void UploadTextureImmutable(GLenum TextureType);
		void UploadTextureImmutable(GLenum TextureType, const char* Filename);

		void UploadTexture(GLenum TextureType);
		void UploadTexture(GLenum TextureType, const char* Filename);
	private:
		uint32_t m_Texture;
		TextureData m_TextureData;
	};
}