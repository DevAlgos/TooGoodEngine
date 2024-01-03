#include <pch.h>
#include "Bucket.h"

namespace Ecs
{
	Bucket::Bucket(std::type_index TypeIndex)
		: m_BucketType(TypeIndex)
	{
	}
	Bucket::Bucket()
		: m_BucketType(typeid(int))
	{
	}
	Bucket::~Bucket()
	{
	}
}