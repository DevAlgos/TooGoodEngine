#include "Application.h"
#include "UserApplication.h"
#include <memory>


namespace TGE
{
	int Main(const UserApplication& UserApp)
	{
		std::shared_ptr<Application> App = std::make_shared<Application>(UserApp);
		App->Run();
		return 0;
	}
}

