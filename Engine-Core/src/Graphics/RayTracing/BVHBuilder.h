#pragma once

#include <glm/glm.hpp>

#include <vector>

#include <ppl.h>
#include <concurrent_vector.h>

#include "Graphics/Buffers.h"
#include "Graphics/ModeImporting/Importer.h"
#include "Utils/Threading/ThreadPool.h"

namespace TooGoodEngine {

	struct AABB
	{
		glm::vec4 MinValue = glm::vec4(FLT_MAX);
		glm::vec4 MaxValue = glm::vec4(FLT_MIN);

		void Grow(const glm::vec4& Val)
		{
			MinValue = glm::min(Val, MinValue);
			MaxValue = glm::max(Val, MaxValue);
		}

		void Grow(const AABB& other)
		{
			MinValue = glm::min(MinValue,other.MinValue);
			MaxValue = glm::max(MaxValue, other.MaxValue);
		}

		float Area() const
		{
			glm::vec4 e = MaxValue - MinValue; 
			return 2.0f * (e.x * e.y + e.x * e.z + e.y * e.z);
		}

		int MaximumExtent() const
		{
			glm::vec4 d = MaxValue - MinValue;
			if (d.x > d.y && d.x > d.z)
				return 0;
			else if (d.y > d.z)
				return 1;
			else
				return 2;
		}

		glm::vec4 Offset(const glm::vec4& p) const
		{
			glm::vec4 o = p - MinValue;
			if (MaxValue.x > MinValue.x) o.x /= MaxValue.x - MinValue.x;
			if (MaxValue.y > MinValue.y) o.y /= MaxValue.y - MinValue.y;
			if (MaxValue.z > MinValue.z) o.z /= MaxValue.z - MinValue.z;
			return o;
		}

		
	};

	struct Triangle
	{
		glm::vec4 Vertices[3];
		glm::vec4 Normal;
		glm::vec4 Centroid;	

		AABB Bounds() const
		{
			AABB b{};
			b.Grow(Vertices[0]);
			b.Grow(Vertices[1]);
			b.Grow(Vertices[2]);

			return b;
		}
	};


	struct BVHNode
	{
		AABB BoundingBox;
		int LeftNode = -1; //-1 represent null
		int RightNode = -1;
		int NumberOfPrims = 0;
		int IsLeaf = false;
		int Primitive[100];

		BVHNode()
		{
			memset(Primitive, -1, 100);
		}
	};

	struct BVHTreeletNode
	{
		BVHNode Node;
		int TreeletIndex = 0;
	};

	struct BucketInfo
	{
		int count = 0;
		AABB bounds;
	};

	struct MortonPrimitive
	{
		uint32_t TriangleIndex;
		uint32_t MortonCode;

		static uint32_t GetRadix(const MortonPrimitive& value) {
			return value.MortonCode;
		}
	};



	struct LBVHTreelet
	{
		uint32_t min;
		uint32_t max;

		std::vector<BVHNode> LocalNodes;
	};

	static uint32_t LeftShift3(uint32_t x) 
	{
		constexpr uint32_t MaxVal = 1 << 10;

		if (x == MaxVal)
			x--;

		x = (x | (x << 16)) &  0b00000011000000000000000011111111;
		x = (x | (x << 8))  &  0b00000011000000001111000000001111;
		x = (x | (x << 4))  &  0b00000011000011000011000011000011;
		x = (x | (x << 2))  &  0b00001001001001001001001001001001;
		return x;
	}

	static uint32_t EncodeMorton(const glm::vec3& Vec)
	{
		/*
		* encodes vec3 to morton by interleaving bits of x, y and z 
		* in the form z3y3x3 z2y2x2 z1y1x1
		*/
		
		return (LeftShift3((uint32_t)Vec.z) << 2) | 
			   (LeftShift3((uint32_t)Vec.y) << 1) | 
			   (LeftShift3((uint32_t)Vec.x));
	}

	template<typename Type>
	using ConcurrentVector = concurrency::concurrent_vector<Type>;


	enum class BuildType 
	{
		MedianSplit = 0, SAHSplit, HLSplit
	};

	class BVHBuilder
	{
	public:
		BVHBuilder();
		~BVHBuilder();

		void AddMesh(const Mesh& mesh, const glm::mat4& Transform, int MaterialIndex);
		void AddModel(const Model& model, const glm::mat4& Transform);

		void Build(BuildType Type);
		void Dispatch(int TriangleBindingIndex, int BVHBindingIndex);

		inline const int GetCurrentNumberOfNodes() const { return m_CurrentNumberOfNodes; }

	private:
		glm::vec4 ComputeCentroid(const Triangle& tri);

		glm::vec4 ComputeMinimum(const Triangle& tri);
		glm::vec4 ComputeMaximum(const Triangle& tri);

		AABB ComputeBounds(int min, int max);


		void SortTriangles(size_t Axis);

		/*
		* Brief:
		* Normal bounding hierarchy structure split at the spatial median
		* built recursively.
		*/

		void BuildBVH(int NodeIndex, int min, int max);

		/*
		* Brief:
		* Builds a bounding hierarchy structure using buckets and calculating
		* probability of ray hitting a AABB of a bucket using surface area heurstics. 
		* Also built recursively.
		*/

		void BuildSAHBVH(int NodeIndex, int min, int max);


		/*
		* Brief:
		* Builds a bounding hierarchy structure using morton primitves
		* which encodes a centroid into a uint32_t by interleaving bits
		* in the form Z3Y3X3 Z2Y2X2 Z1Y1X1 these bits can now be sorted 
		* using radix sort. After this the root nodes of each treelet
		* is added to the main Node structure using surface area heurstics.
		*/

		void BuildHLBVH();

		void BuildHLBVHNode(
			LBVHTreelet& NodeToBuild, 
			const ConcurrentVector<MortonPrimitive>& Prims,
			int Start, int End, int nPrimitives, int BitIndex, int NodeIndex);

		void BuildUpperSAH(
			ConcurrentVector<BVHTreeletNode>& Nodes, 
			ConcurrentVector<LBVHTreelet>& Treelets,
			int NodeIndex, 
			int min, 
			int max);

	private:
		ConcurrentVector<Triangle> m_TriangleData;
		std::vector<BVHNode> m_Nodes;

		size_t m_TriangleBufferSize = sizeof(Triangle) * 100;
		size_t m_NodeBufferSize = sizeof(BVHNode) * 100;

		OpenGLBuffer m_TriangleBuffer;
		OpenGLBuffer m_NodeBuffer;	

		size_t m_Axis = 0; 
		size_t m_DebugCount = 0;

		int m_CurrentNumberOfNodes = 0; //debug
	};

}