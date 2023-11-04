#include <pch.h>
#include "DebuggingLayer.h"


namespace
{
	Utils::MemoryData s_MemoryData;
}


#ifdef _DEBUG

void* operator new(size_t size)
{
	s_MemoryData.AllocatedMemory += size;
	return malloc(size);
}

void operator delete(void* block, size_t size)
{
	s_MemoryData.FreedMemory += size;
	return free(block);
}

#endif

namespace Utils
{
	void DebuggingLayer::OnInit()
	{
		
	}

	void DebuggingLayer::OnUpdate() 
	{
		
	}

	void DebuggingLayer::OnGUIUpdate() 
	{

		if (ImGui::Begin("Debug Window"))
		{
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
#ifdef _DEBUG
			ImGui::Text("Memory Using: %i", s_MemoryData.CurrentlyUsingMemory());
#endif
			ImGui::End();
		}
	
	}

	void DebuggingLayer::OnShutdown()
	{
	}
}