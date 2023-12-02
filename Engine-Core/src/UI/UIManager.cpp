#include <pch.h>

#include <ft2build.h>
#include <freetype/freetype.h>

#include "UIManager.h"

namespace TGE
{
	UIManager::UIManager()
		: m_FaceCount(1)
	{
		FT_Error error = FT_Init_FreeType(&m_FtLibrary);
		if (error)
			LOGERROR("FT library failed to init");

		m_Faces.emplace_back(); //null face
	}

	UIManager::~UIManager()
	{
		FT_Done_FreeType(m_FtLibrary);
	}

	uint32_t UIManager::LoadFont(const std::string_view& FileLocation)
	{
		m_Faces.emplace_back();
		FT_Face& FontFace = m_Faces.back().FontFace;

		FT_Error error = FT_New_Face(m_FtLibrary, FileLocation.data(), 0, &FontFace);
		if (error)
		{
			LOGERROR("Font failed to load file type or location");
			return UINT32_MAX;
		}
		
		error = FT_Set_Pixel_Sizes(FontFace, 0, 32); //32x32 pixels

		TextureData TexData;
		TexData.InternalFormat = TextureFormat::RGBA8;
		TexData.TextureParamaters = 
		{
			{GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
			{GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
			{GL_TEXTURE_MIN_FILTER, GL_NEAREST},
			{GL_TEXTURE_MAG_FILTER, GL_NEAREST}
		};

		

		for (uint8_t character = 0; character < 128; character++)
		{
			FT_UInt glyphIndex = FT_Get_Char_Index(FontFace, character);

			if (FT_Load_Glyph(FontFace, glyphIndex, FT_LOAD_RENDER))
			{
				LOGERROR("Failed to load glyph");
				continue;
			}
			
			if (FontFace->glyph->bitmap.width == 0 || FontFace->glyph->bitmap.rows == 0)
			{
				std::string msg = "Skipped character " + std::string(1, character);
				LOGWARNING(msg);
				continue;
			}

			uint32_t* Buffer = new uint32_t[FontFace->glyph->bitmap.width *
											FontFace->glyph->bitmap.rows];

			for (uint32_t i = 0; i < FontFace->glyph->bitmap.width * FontFace->glyph->bitmap.rows; i++)
			{
				uint8_t R = FontFace->glyph->bitmap.buffer[i];
				Buffer[i] = ConvertToRGBA(R, 0x00, 0x00, 0xFF);
			}

			TexData.Width = FontFace->glyph->bitmap.width;
			TexData.Height = FontFace->glyph->bitmap.rows;

			m_Faces.back().Characters.insert({ character, Character(TexData, Buffer,
														  { FontFace->glyph->bitmap.width,
															FontFace->glyph->bitmap.rows},
														  { FontFace->glyph->bitmap_left,
															FontFace->glyph->bitmap_top},
															FontFace->glyph->advance.x) });

			delete[] Buffer;
			
		}

		FT_Done_Face(FontFace);

		return m_FaceCount++;
	}

	uint32_t UIManager::LoadFont(const std::string_view& FileLocation, uint32_t Index)
	{
		m_Faces.emplace_back();
		FT_Face& FontFace = m_Faces.back().FontFace;

		FT_Error error = FT_New_Face(m_FtLibrary, FileLocation.data(), Index, &FontFace);

		if (error)
		{
			LOGERROR("Font failed to load file type or location");
			return UINT32_MAX;
		}

		error = FT_Set_Pixel_Sizes(FontFace, 0, 32); //32x32 pixels

		TextureData TexData;
		TexData.InternalFormat = TextureFormat::RGBA8;
		TexData.TextureParamaters =
		{
			{GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
			{GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
			{GL_TEXTURE_MIN_FILTER, GL_NEAREST},
			{GL_TEXTURE_MAG_FILTER, GL_NEAREST}
		};



		for (uint8_t character = 0; character < 128; character++)
		{
			if (FT_Load_Char(FontFace, character, FT_LOAD_RENDER))
			{
				LOGERROR("Failed to load glyph");
				continue;
			}

			uint32_t* Buffer = new uint32_t[FontFace->glyph->bitmap.width *
				FontFace->glyph->bitmap.rows];

			for (uint32_t i = 0; i < FontFace->glyph->bitmap.width * FontFace->glyph->bitmap.rows; i++)
			{
				uint8_t R = FontFace->glyph->bitmap.buffer[i];
				Buffer[i] = ConvertToRGBA(R, 0x00, 0x00, 0xFF);
			}

			TexData.Width = FontFace->glyph->bitmap.width;
			TexData.Height = FontFace->glyph->bitmap.rows;

			m_Faces.back().Characters.insert({ character, Character(TexData, Buffer,
														  { FontFace->glyph->bitmap.width,
															FontFace->glyph->bitmap.rows},
														  { FontFace->glyph->bitmap_left,
															FontFace->glyph->bitmap_top},
															FontFace->glyph->advance.x) });

			delete[] Buffer;
		}

		FT_Done_Face(FontFace);
		return m_FaceCount++;
	}

	const Character& UIManager::GetCharacter(uint32_t FontIndex, uint8_t character) const
	{
		if (FontIndex >= m_FaceCount)
		{
			LOGWARNING("Font index out of range null font returned");
			return m_Faces[0].Characters.at(0);
		}

		return m_Faces[FontIndex].Characters.at(character);
	}
	
}