#pragma once

#include <freetype/freetype.h>
#include <Graphics/Texture.h>

#include <iostream>
#include <map>
#include <vector>

namespace TooGoodEngine
{
	const uint32_t GetMapWidth();
	const uint32_t GetMapHeight();
	const uint32_t GetPixelPerChar();

	struct Character
	{

		Character(glm::ivec2 coordinate, const glm::vec2& size, const glm::vec2& bearing, uint32_t advance)
			: Coordinate(coordinate), Size(size), Bearing(bearing), Advance(advance)
		{
		}

		glm::ivec2 Coordinate;
		glm::vec2 Size;
		glm::vec2 Bearing;
		uint32_t Advance;
	};

	struct FontData
	{
		FontData(TextureData data, uint32_t* Buffer)
			: CharacterSheet(Buffer, data)
		{
		}

		FT_Face FontFace = nullptr;
		Texture CharacterSheet;
		std::map<char, Character> Characters;
	};

	class UIManager
	{
	public:
		UIManager();
		~UIManager();

		uint32_t LoadFont(const std::string_view& FileLocation, uint32_t Index = 0);

		const uint32_t GetFontCount() const { return m_FaceCount; }
		const Character& GetCharacter(uint32_t FontIndex, uint8_t character) const;
		const FontData& GetFont(uint32_t FontIndex) const;

	private:

		uint32_t ConvertToRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
		{
			return (static_cast<uint32_t>(r) |
					static_cast<uint32_t>(g << 8) |
					static_cast<uint32_t>(b << 16) |
					static_cast<uint32_t>(a << 24));
		}
	private:
		FT_Library m_FtLibrary;
		std::vector<FontData> m_Faces;
		uint32_t m_FaceCount;
	};
}