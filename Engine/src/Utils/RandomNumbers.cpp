#include <pch.h>
#include "RandomNumbers.h"

namespace
{
    static    uint32_t CurrentSeed = 1;
    constexpr uint32_t UIntMax = UINT32_MAX;
}

namespace Utils
{
	uint32_t PCGHash(uint32_t seed)
	{
        uint32_t state = seed * 747796405u + 2891336453u;
        uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
        return (word >> 22u) ^ word;
	}

    float GenFloat(float min, float max)
    {
        CurrentSeed = PCGHash(CurrentSeed);

        return min + ((float)CurrentSeed / (float)UIntMax * (max-min));
    }

    uint32_t GenUInt(uint32_t min, uint32_t max)
    {
        CurrentSeed = PCGHash(CurrentSeed);

        return min + (CurrentSeed / UIntMax) * (max - min);
    }

    int GenInt(int min, int max)
    {
        CurrentSeed = PCGHash(CurrentSeed);

        return min + ((int)CurrentSeed / (int)UIntMax * (max - min));
    }

    uint16_t GenUShort(uint16_t min, uint16_t max)
    {
        CurrentSeed = PCGHash(CurrentSeed);

        return min + ((uint16_t)CurrentSeed / (uint16_t)UIntMax * (max - min));
    }

    short GenShort(short min, short max)
    {
        CurrentSeed = PCGHash(CurrentSeed);

        return min + ((short)CurrentSeed / (short)UIntMax * (max-min));
    }

    uint8_t GenUChar(uint8_t min, uint8_t max)
    {
        CurrentSeed = PCGHash(CurrentSeed);

        return min + ((uint8_t)CurrentSeed / (uint8_t)UIntMax * (max-min));
    }

    char GenChar(char min, char max)
    {
        CurrentSeed = PCGHash(CurrentSeed);

        return min + ((char)CurrentSeed / (char)UIntMax * (max-min));
    }

}