#pragma once
#include <memory>

#include "EngineLogger.h"


namespace TooGoodEngine {
	class Log
	{
	public:
		static void Init();
		static std::shared_ptr<EngineLogger>& GetEngineLogger() { return s_MainEngineLogger; }

	private:
		static std::shared_ptr<EngineLogger> s_MainEngineLogger;
	};
}

#define TGE_LOG_INFO(...)		TooGoodEngine::Log::GetEngineLogger()->Log(TooGoodEngine::TextColor::MAGENTA, "[Core] ",       __VA_ARGS__)
#define TGE_LOG_WARN(...)		TooGoodEngine::Log::GetEngineLogger()->Log(TooGoodEngine::TextColor::YELLOW,  "[Core-Warn] ",  __VA_ARGS__)
#define TGE_LOG_ERROR(...)		TooGoodEngine::Log::GetEngineLogger()->Log(TooGoodEngine::TextColor::RED,     "[Core-Error] ", __VA_ARGS__)

#define TGE_CLIENT_LOG(...)	  TooGoodEngine::Log::GetEngineLogger()->ClientLog(TooGoodEngine::LogColor::Core, "[Client] ",        __VA_ARGS__)
#define TGE_CLIENT_WARN(...)  TooGoodEngine::Log::GetEngineLogger()->ClientLog(TooGoodEngine::LogColor::Warn, "[Client-Warn] ",   __VA_ARGS__)
#define TGE_CLIENT_ERROR(...) TooGoodEngine::Log::GetEngineLogger()->ClientLog(TooGoodEngine::LogColor::Error, "[Client-Error] ",   __VA_ARGS__)

#ifdef _WIN32
	#define TGE_HALT() __debugbreak()
#else
	#define TGE_HALT()
#endif

//TODO: remove _DEBUG_WITH_PYTHON build option

#ifdef _DEBUG 
	#define TGE_ASSERT(condition, ...) if(!(condition)) {TGE_LOG_ERROR(__VA_ARGS__); TGE_HALT();}
#else
	#define TGE_ASSERT(condition, ...) 
#endif

#define TGE_FORCE_ASSERT(condition, ...) if(!(condition)) {TGE_LOG_ERROR(__VA_ARGS__); TGE_HALT();}