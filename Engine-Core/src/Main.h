#include "Application.h"
#include "UserApplication.h"
#include <memory>

namespace TooGoodEngine
{
	int Main(const UserApplication& UserApp)
	{
		Application App(UserApp); 
		App.Run();
		return 0;
	}
}

