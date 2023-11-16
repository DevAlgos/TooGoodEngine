#pragma once

#include <iostream>

/*
Note, these random numbers are not meant for cryptography or anything which 
involves security so i do not recommend you use this for that, however for 
games it produces very realistic random numbers and is performance friendly,
information on this algorithm: https://www.reedbeta.com/blog/quick-and-easy-gpu-random-numbers-in-d3d11/
*/

namespace Utils
{
	uint32_t PCGHash(uint32_t seed); //Generates pseudorandom number based on seed given
	
	float    GenFloat(float min, float max);

	uint32_t GenUInt(uint32_t min, uint32_t max);
	int      GenInt(int min, int max);

	uint16_t GenUShort(uint16_t min, uint16_t max);
	short    GenShort(short min, short max);

	uint8_t  GenUChar(uint8_t min, uint8_t max);
	char     GenChar(char min, char max);

}