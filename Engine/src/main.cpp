#include <pch.h>

#ifdef _DEBUG
static MemoryData MemoryStats;

void* operator new(size_t size)
{
	MemoryStats.AllocatedMemory += size;
	return malloc(size);
}

void operator delete(void* block, size_t size)
{
	MemoryStats.FreedMemory += size;
	return free(block);
}

#endif



int main()
{
	std::unique_ptr<Application> App = std::make_unique<Application>();
#ifdef _DEBUG
	App->MainLoop(MemoryStats);
#else
	App->MainLoop();
#endif

	
	
}