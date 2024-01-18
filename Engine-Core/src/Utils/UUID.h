#pragma once

#include "BigInt.h"

namespace TooGoodEngine {

	class UUID
	{
	public:
		UUID();
		UUID(const uint128_t& id);
		~UUID() = default;

		const uint128_t GetID() const { return m_ID; }

	private:
		uint128_t m_ID;
	};
}