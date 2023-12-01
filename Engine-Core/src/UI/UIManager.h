#pragma once

#include <freetype/freetype.h>

#include <iostream>
#include <map>
#include <vector>

namespace TGE
{
	struct Character
	{

	};

	struct FontData
	{
		FT_Face FontFace;
		std::map<char, Character> Characters;
	};

	class UIManager
	{
	public:
		UIManager();
		~UIManager();

		[[nodiscard]] uint32_t LoadFont(const std::string_view& FileLocation) {};
		[[nodiscard]] uint32_t LoadFont(const std::string_view& FileLocation, uint32_t Index) {};

	private:
		FT_Library m_FtLibrary;
		std::vector<FontData> m_Faces;
		uint32_t m_FaceCount;
	};
}