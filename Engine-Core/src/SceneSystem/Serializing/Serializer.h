#pragma once

#include <nlohmann/json.hpp>
#include <filesystem>

namespace TooGoodEngine {
	
	using json = nlohmann::json;

	class Serializer
	{
	public:
		Serializer() = delete;
		~Serializer() = delete;

		template<class Type>
		bool Serialize(const std::filesystem::path& path, const Type& type);
		
	};

	template<class Type>
	inline bool Serializer::Serialize(const std::filesystem::path& path, const Type& type)
	{
		TGE_LOG_WARN("Not a valid type to serialize");
		return false;
	}

	

}