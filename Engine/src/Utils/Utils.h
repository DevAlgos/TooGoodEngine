#pragma once

struct MemoryData
{
	
	size_t AllocatedMemory;
	size_t FreedMemory;

	size_t CurrentlyUsingMemory() { return AllocatedMemory - FreedMemory; }
};


