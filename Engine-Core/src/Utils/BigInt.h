#pragma once

#include <cinttypes>
#include <sstream>

namespace TooGoodEngine {
	/*
	* helper for UUID, whilst it is extremely unlikley that 64 bit uuid will collide
	* (almost impossible statisically) i am using a 128 bit integer to decrease that
	* likleyhood
	*/
	struct uint128_t
	{
	public:
		uint128_t(uint64_t lower, uint64_t upper) : m_Lower(lower), m_Upper(upper) {}
		uint128_t(const uint128_t&) = default;
		uint128_t() : m_Lower(0U), m_Upper(0U) {}
		~uint128_t() = default;

		uint128_t& operator=(const uint128_t& other)
		{
			m_Lower = other.m_Lower;
			m_Upper = other.m_Upper;

			return *this;
		}

		bool operator!=(const uint128_t& other)
		{
			return m_Lower != other.m_Lower || m_Upper != other.m_Upper;
		}

		bool operator==(const uint128_t& other)
		{
			return m_Lower == other.m_Lower && m_Upper == other.m_Upper;
		}

		bool operator>(const uint128_t& other)
		{
			if (m_Upper == other.m_Upper)
				return m_Lower > other.m_Lower;

			return m_Upper > other.m_Upper;
		}

		bool operator>=(const uint128_t& other)
		{
			if (m_Upper == other.m_Upper)
				return m_Lower >= other.m_Lower;

			return m_Upper >= other.m_Upper;
		}

		bool operator<(const uint128_t& other)
		{
			if (m_Upper == other.m_Upper)
				return m_Lower < other.m_Lower;

			return m_Upper < other.m_Upper;
		}

		bool operator<=(const uint128_t& other)
		{
			if (m_Upper == other.m_Upper)
				return m_Lower <= other.m_Lower;

			return m_Upper <= other.m_Upper;
		}

		uint128_t& operator+(const uint128_t& other)
		{
			m_Lower += other.m_Lower;
			m_Upper += other.m_Upper;

			return *this;
		};

		uint128_t& operator-(const uint128_t& other)
		{
			m_Lower -= other.m_Lower;
			m_Upper -= other.m_Upper;

			return *this;
		}

		uint128_t& operator+=(const uint128_t& other)
		{
			m_Lower += other.m_Lower;
			m_Upper += other.m_Upper;

			return *this;
		};

		uint128_t& operator-=(const uint128_t& other)
		{
			m_Lower -= other.m_Lower;
			m_Upper -= other.m_Upper;

			return *this;
		}

		uint128_t& operator*(const uint128_t& other)
		{
			m_Lower *= other.m_Lower;
			m_Upper *= other.m_Upper;

			return *this;
		};

		uint128_t& operator*=(const uint128_t& other)
		{
			m_Lower *= other.m_Lower;
			m_Upper *= other.m_Upper;

			return *this;
		}

		uint128_t& operator/(const uint128_t& other)
		{
			m_Lower /= other.m_Lower;
			m_Upper /= other.m_Upper;

			return *this;
		};

		uint128_t& operator/=(const uint128_t& other)
		{
			m_Lower /= other.m_Lower;
			m_Upper /= other.m_Upper;

			return *this;
		}

		std::string ToString() const
		{
			std::ostringstream ss;
			ss << m_Lower << m_Upper;
			return ss.str();
		}

	private:
		uint64_t m_Lower;
		uint64_t m_Upper;
	};
}