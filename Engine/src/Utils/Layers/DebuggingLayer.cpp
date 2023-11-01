#include <pch.h>
#include "DebuggingLayer.h"


namespace
{
	Utils::MemoryData s_MemoryData;
}

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
			ImGui::Text("Memory Allocated %i", s_MemoryData.AllocatedMemory);
			ImGui::Text("Memory Freed %i", s_MemoryData.FreedMemory);
			ImGui::Text("Memory Using: %i", s_MemoryData.CurrentlyUsingMemory());
			ImGui::End();
		}
	
	}

	void DebuggingLayer::OnShutdown()
	{
	}
}