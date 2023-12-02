#pragma once

#include <freetype/freetype.h>
#include <Graphics/Texture.h>

#include <iostream>
#include <map>
#include <vector>

namespace TGE
{
	struct Character
	{

		Character(TextureData data, uint32_t* Buffer, const glm::vec2& size, const glm::vec2& bearing, uint32_t advance)
			: CharacterGlyph(Buffer, data), Size(size), Bearing(bearing), Advance(advance)
		{
		}

		Texture CharacterGlyph;
		glm::vec2 Size;
		glm::vec2 Bearing;
		uint32_t Advance;
	};

	struct FontData
	{
		FT_Face FontFace = nullptr;
		std::map<char, Character> Characters;
	};

	class UIManager
	{
	public:
		UIManager();
		~UIManager();

		_NODISCARD uint32_t LoadFont(const std::string_view& FileLocation);
		_NODISCARD uint32_t LoadFont(const std::string_view& FileLocation, uint32_t Index);

		_NODISCARD const Character& GetCharacter(uint32_t FontIndex, uint8_t character) const;

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