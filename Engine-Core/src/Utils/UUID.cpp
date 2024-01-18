#include "pch.h"
#include "UUID.h"

#include <random>

namespace TooGoodEngine {

	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_RandomEngine(s_RandomDevice());

	static std::uniform_int_distribution<uint64_t> s_Distribution;

	UUID::UUID()
		: m_ID(s_Distribution(s_RandomEngine), s_Distribution(s_RandomEngine))
	{
	}
	UUID::UUID(const uint128_t& id)
		: m_ID(id)
	{
	}
}