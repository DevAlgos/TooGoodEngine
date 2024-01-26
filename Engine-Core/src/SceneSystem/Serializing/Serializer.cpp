#include "pch.h"
#include "Serializer.h"

#include "Utils/Log.h"

namespace TooGoodEngine {
	
	static std::unordered_map<std::filesystem::path, json> s_CachedFiles;

	template<>
	bool Serializer::Serialize<Ecs::TransformComponent>(const std::filesystem::path& path, const Ecs::TransformComponent& Component)
	{
		TGE_LOG_WARN("Not a valid type to serialize");
		return true;
	}

	template<>
	bool Serializer::Serialize<Ecs::MaterialComponent>(const std::filesystem::path& path, const Ecs::MaterialComponent& Component)
	{
		TGE_LOG_WARN("Not a valid type to serialize");
		return true;
	}

	template<>
	bool Serializer::Serialize<Ecs::QuadComponent>(const std::filesystem::path& path, const Ecs::QuadComponent& Component)
	{
		TGE_LOG_WARN("Not a valid type to serialize");
		return true;
	}
}