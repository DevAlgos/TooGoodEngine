#include <pch.h>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftbitmap.h>

#include "UIManager.h"

constexpr uint32_t FONTMAP_WIDTH =  1024U;
constexpr uint32_t FONTMAP_HEIGHT = 1024U;
constexpr uint32_t PIXEL_PER_CHAR = 64U;


namespace tge
{
	UIManager::UIManager()
		: m_FaceCount(1)
	{
		FT_Error error = FT_Init_FreeType(&m_FtLibrary);
		if (error)
			LOGERROR("FT library failed to init");

		m_Faces.reserve(32);
	}

	UIManager::~UIManager()
	{
		FT_Done_FreeType(m_FtLibrary);
	}


	uint32_t UIManager::LoadFont(const std::string_view& FileLocation, uint32_t Index)
	{
		TextureData TexData;
		TexData.InternalFormat = TextureFormat::RGBA8;
		TexData.Type = TextureType::Texture2D;
		TexData.Width = FONTMAP_WIDTH;
		TexData.Height = FONTMAP_HEIGHT;
		TexData.TextureParamaters =
		{
			{GL_TEXTURE_MIN_FILTER, GL_NEAREST},
			{GL_TEXTURE_MAG_FILTER, GL_NEAREST},
			{GL_TEXTURE_WRAP_S,		GL_CLAMP_TO_BORDER},
			{GL_TEXTURE_WRAP_T,		GL_CLAMP_TO_BORDER}
		};

		uint32_t* TextureBuffer = new uint32_t[FONTMAP_WIDTH * FONTMAP_HEIGHT];
		for (uint32_t i = 0; i < FONTMAP_WIDTH * FONTMAP_HEIGHT; i++)
			TextureBuffer[i] = 0xFF000000;

		m_Faces.emplace_back(TexData, TextureBuffer);
		FT_Face& FontFace = m_Faces.back().FontFace;

		FT_Error error = FT_New_Face(m_FtLibrary, FileLocation.data(), Index, &FontFace);
		if (error)
		{
			LOGERROR("Font failed to load file type or location");
			return UINT32_MAX;
		}

		error = FT_Set_Pixel_Sizes(FontFace, PIXEL_PER_CHAR, PIXEL_PER_CHAR);

		uint32_t CurrentXCoord = 0;
		uint32_t CurrentYCoord = 0;

		for (uint8_t character = 32; character < 127; character++)
		{
			FT_UInt glyphIndex = FT_Get_Char_Index(FontFace, character);

			if (FT_Load_Glyph(FontFace, glyphIndex, FT_LOAD_RENDER))
			{
				LOGERROR("Failed to load glyph");
				continue;
			}

			if (FontFace->glyph->bitmap.width == 0 || FontFace->glyph->bitmap.rows == 0)
			{
				std::string msg = "Skipped character " + std::to_string(character);
				LOGWARNING(msg);
				continue;
			}

			FT_Bitmap* bitmap = &FontFace->glyph->bitmap;


			uint32_t* Buffer = new uint32_t[PIXEL_PER_CHAR * PIXEL_PER_CHAR];

			for (uint32_t y = 0; y < PIXEL_PER_CHAR; ++y)
			{
				for (uint32_t x = 0; x < PIXEL_PER_CHAR; ++x)
				{
					uint8_t val = 0;
					if (x < bitmap->width && y < bitmap->rows)
						val = bitmap->buffer[x + y * bitmap->width];

					Buffer[(PIXEL_PER_CHAR - 1 - y) + x * PIXEL_PER_CHAR] = ConvertToRGBA(val, val, val, val);
				}
			}


			uint32_t PixelXCoord = CurrentXCoord * PIXEL_PER_CHAR;
			uint32_t PixelYCoord = CurrentYCoord * PIXEL_PER_CHAR;
			uint32_t i = 0;

			for (uint32_t x = PixelXCoord; x < PixelXCoord + PIXEL_PER_CHAR && x < FONTMAP_WIDTH; ++x) {
				for (uint32_t y = PixelYCoord; y < PixelYCoord + PIXEL_PER_CHAR && y < FONTMAP_HEIGHT; ++y) {
					TextureBuffer[x + y * FONTMAP_WIDTH] = Buffer[i++];
				}
			}


			m_Faces.back().Characters.insert({ character, Character({CurrentXCoord,CurrentYCoord},
														  { FontFace->glyph->bitmap.width,
															FontFace->glyph->bitmap.rows},
														  { FontFace->glyph->bitmap_left,
															FontFace->glyph->bitmap_top},
															FontFace->glyph->advance.x) });

			delete[] Buffer;
			CurrentXCoord++;

			if (CurrentXCoord * PIXEL_PER_CHAR >= FONTMAP_WIDTH)
			{
				CurrentXCoord = 0;
				CurrentYCoord++;
			}

		}

		m_Faces.back().CharacterSheet.SetData(TextureBuffer);

		delete[] TextureBuffer;
		FT_Done_Face(FontFace);

		return m_FaceCount++ - 1;
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

	const FontData& UIManager::GetFont(uint32_t FontIndex) const
	{
		if (FontIndex >= m_FaceCount)
		{
			LOGWARNING("Font index out of range null font returned");
			return m_Faces[0];
		}

		return m_Faces[FontIndex];
	}
	
	const uint32_t GetMapWidth()
	{
		return FONTMAP_WIDTH;
	}

	const uint32_t GetMapHeight()
	{
		return FONTMAP_HEIGHT;
	}

	const uint32_t GetPixelPerChar()
	{
		return PIXEL_PER_CHAR;
	}

}