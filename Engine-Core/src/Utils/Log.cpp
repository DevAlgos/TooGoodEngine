#include "pch.h"
#include "Log.h"


namespace TooGoodEngine {

	std::shared_ptr<EngineLogger> Log::s_MainEngineLogger;

	void Log::Init()
	{
		s_MainEngineLogger = std::make_shared<EngineLogger>();
	}
}
