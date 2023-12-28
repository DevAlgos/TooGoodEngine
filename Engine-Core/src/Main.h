#include "Application.h"
#include "UserApplication.h"
#include <memory>
#include <Python.h>

namespace tge
{
	int Main(const UserApplication& UserApp)
	{
		std::unique_ptr<Application> App = std::make_unique<Application>(UserApp);
		App->Run();
		return 0;
	}
}

