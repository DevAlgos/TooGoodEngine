#include <pch.h>

#include <ft2build.h>
#include <freetype/freetype.h>

#include "UIManager.h"

namespace TGE
{
	UIManager::UIManager()
		: m_FaceCount(0)
	{
		FT_Error error = FT_Init_FreeType(&m_FtLibrary);
		if (error != 0)
			LOGERROR("FT library failed to init");
	}

	UIManager::~UIManager()
	{
		FT_Done_FreeType(m_FtLibrary);
	}
	
}