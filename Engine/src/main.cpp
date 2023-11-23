#include <pch.h>

int main()
{
	std::unique_ptr<Application> App = std::make_unique<Application>();

	App->MainLoop();	
}