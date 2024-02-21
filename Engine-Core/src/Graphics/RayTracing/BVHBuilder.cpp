#include "pch.h"
#include "BVHBuilder.h"

#include <pplawait.h>

namespace TooGoodEngine {
	
	BVHBuilder::BVHBuilder()
		: m_TriangleBuffer(BufferType::ShaderStorageBuffer, {nullptr, m_TriangleBufferSize, GL_DYNAMIC_DRAW}), 
		  m_NodeBuffer(BufferType::ShaderStorageBuffer, {nullptr, m_NodeBufferSize, GL_DYNAMIC_DRAW})
	{
		m_CachedIndicesIndex.push_back(0);
		m_CachedIndicesIndex.push_back(3);
		m_CachedIndicesIndex.push_back(6);
	}

	BVHBuilder::~BVHBuilder()
	{
		
	}
	 
	void BVHBuilder::AddMesh(const Mesh& mesh, const glm::mat4& Transform)
	{
		constexpr size_t VertexSize = 8;

		for (size_t i = m_CachedIndicesIndex.back(); i < mesh.Indicies.size(); i += 3)
			m_CachedIndicesIndex.push_back(i);

		size_t LastIndex = std::min(m_CachedIndicesIndex.size(), mesh.Indicies.size() / 3);

		auto ProcessMesh = [&](size_t i) 
			{
				uint32_t Index1 = mesh.Indicies[i];
				uint32_t Index2 = mesh.Indicies[i + 1];
				uint32_t Index3 = mesh.Indicies[i + 2];

				size_t offset1 = Index1 * VertexSize;
				size_t offset2 = Index2 * VertexSize;
				size_t offset3 = Index3 * VertexSize;

				glm::vec4 Vertex1(mesh.Vertices[offset1], mesh.Vertices[offset1 + 1], mesh.Vertices[offset1 + 2], 1.0f);
				glm::vec4 Vertex2(mesh.Vertices[offset2], mesh.Vertices[offset2 + 1], mesh.Vertices[offset2 + 2], 1.0f);
				glm::vec4 Vertex3(mesh.Vertices[offset3], mesh.Vertices[offset3 + 1], mesh.Vertices[offset3 + 2], 1.0f);

				Vertex1 = Transform * Vertex1;
				Vertex2 = Transform * Vertex2;
				Vertex3 = Transform * Vertex3;
				
				glm::vec4 Normal = glm::vec4(glm::cross(glm::normalize(glm::vec3(Vertex2 - Vertex1)), 
											            glm::normalize(glm::vec3(Vertex3 - Vertex1))), 1.0); 

				Triangle tri{};
				tri.Vertices[0] = Vertex1;
				tri.Vertices[1] = Vertex2;
				tri.Vertices[2] = Vertex3;
				tri.Normal = glm::normalize(Normal);
				tri.Centroid = ComputeCentroid(tri);


				m_TriangleData.push_back(tri);
			};

		concurrency::parallel_for_each(
			m_CachedIndicesIndex.begin(),
			m_CachedIndicesIndex.begin() + LastIndex,
			ProcessMesh);
	}

	void BVHBuilder::AddModel(const Model& model, const glm::mat4& Transform)
	{
		for (size_t i = 0; i < model.MeshList.size(); i++)
			AddMesh(model.MeshList[i], Transform);
	}

	void BVHBuilder::Build(BuildType Type)
	{
		

		if (m_TriangleData.size() == 0)
			return;

		BVHNode Root;

		{
			Utils::TimedScope BuildSAH("Building BVH time");
			switch (Type)
			{
			case BuildType::MedianSplit:
				
				Root.BoundingBox = ComputeBounds(0, m_TriangleData.size() - 1);

				m_Nodes.push_back(Root);

				BuildBVH(0, 0, m_TriangleData.size() - 1);

				break;

			case BuildType::SAHSplit:
				
				Root.BoundingBox = ComputeBounds(0, m_TriangleData.size() - 1);

				m_Nodes.push_back(Root);

				BuildSAHBVH(0, 0, m_TriangleData.size() - 1);

				break;
			case BuildType::HLSplit:
				BuildHLBVH();
				break;
			default:
				break;
			}

			
		}


		if (m_NodeBufferSize <= m_Nodes.size() * sizeof(BVHNode))
		{
			m_NodeBufferSize = m_Nodes.size() * sizeof(BVHNode) * 2;
			m_NodeBuffer.Resize(m_NodeBufferSize);
		}

		if (m_TriangleData.size() * sizeof(Triangle) >= m_TriangleBufferSize)
		{
			m_TriangleBufferSize = m_TriangleData.size() * sizeof(Triangle) * 2;
			m_TriangleBuffer.Resize(m_TriangleBufferSize);
		}

		{
			Triangle* TriData = (Triangle*)m_TriangleBuffer.Map();
			std::vector<Triangle> LocalCopy(m_TriangleData.begin(), m_TriangleData.end());
			memmove(TriData, LocalCopy.data(), m_TriangleData.size() * sizeof(Triangle));
			m_TriangleBuffer.UnMap();

			BVHNode* nodes = (BVHNode*)m_NodeBuffer.Map();
			memmove(nodes, m_Nodes.data(), m_Nodes.size() * sizeof(BVHNode));
			m_NodeBuffer.UnMap();

		}

		m_TriangleData.clear();
		m_Nodes.clear();
	}

	void BVHBuilder::Dispatch(int TriangleBindingIndex, int BVHBindingIndex)
	{
		m_TriangleBuffer.BindBase(TriangleBindingIndex);
		m_NodeBuffer.BindBase(BVHBindingIndex);
	}

	glm::vec4 BVHBuilder::ComputeCentroid(const Triangle& tri)
	{
		return (tri.Vertices[0] + tri.Vertices[1] + tri.Vertices[2]) / 3.0f;
	}

	glm::vec4 BVHBuilder::ComputeMinimum(const Triangle& tri)
	{
		return glm::min(glm::min(tri.Vertices[0], tri.Vertices[1]), tri.Vertices[2]);
	}

	glm::vec4 BVHBuilder::ComputeMaximum(const Triangle& tri)
	{
		return glm::max(glm::max(tri.Vertices[0], tri.Vertices[1]), tri.Vertices[2]);;
	}

	AABB BVHBuilder::ComputeBounds(int min, int max)
	{
		AABB BoundingBox{};

		BoundingBox.MinValue = glm::vec4(FLT_MAX);
		BoundingBox.MaxValue = glm::vec4(FLT_MIN);

		for (size_t i = min; i < max; i++)
		{
			Triangle& tri = m_TriangleData[i];

			for (size_t a = 0; a < 3; a++)
			{
				glm::vec4& vertex = tri.Vertices[a];

				BoundingBox.MinValue.x = glm::min(BoundingBox.MinValue.x, vertex.x);
				BoundingBox.MinValue.y = glm::min(BoundingBox.MinValue.y, vertex.y);
				BoundingBox.MinValue.z = glm::min(BoundingBox.MinValue.z, vertex.z);

				BoundingBox.MaxValue.x = glm::max(BoundingBox.MaxValue.x, vertex.x);
				BoundingBox.MaxValue.y = glm::max(BoundingBox.MaxValue.y, vertex.y);
				BoundingBox.MaxValue.z = glm::max(BoundingBox.MaxValue.z, vertex.z);
			}
		}

		return BoundingBox;
	}

	void BVHBuilder::SortTriangles(size_t Axis)
	{
		auto CompareTriangles = [this, Axis](const Triangle& tri, const Triangle& tri1)
			{
				return this->ComputeCentroid(tri)[Axis] < this->ComputeCentroid(tri1)[Axis];
			};

		std::sort(std::execution::par_unseq, 
				  m_TriangleData.begin(),  
				  m_TriangleData.end(), CompareTriangles);
	}

	void BVHBuilder::BuildBVH(int NodeIndex, int min, int max)
	{
		if (max - min <= 36)
		{
			BVHNode leafNode;
			leafNode.BoundingBox = ComputeBounds(min, max);

			size_t k = 0;
			for (size_t i = min; i < max; i++)
			{
				leafNode.Primitive[k++] = i;
				leafNode.NumberOfPrims++;
			}

			leafNode.IsLeaf = true;
			m_Nodes.push_back(leafNode);
			m_Nodes[NodeIndex].RightNode = m_Nodes.size() - 1;
			
			return;
		}

		int median = (min + max) / 2;

		BVHNode LeftChild;
		LeftChild.BoundingBox = ComputeBounds(min, median);

		m_Nodes.push_back(LeftChild);
		m_Nodes[NodeIndex].LeftNode = m_Nodes.size() - 1;
		

		BVHNode RightChild;
		RightChild.BoundingBox = ComputeBounds(median, max);

		m_Nodes.push_back(RightChild);
		m_Nodes[NodeIndex].RightNode = m_Nodes.size() - 1;

		BuildBVH(m_Nodes[NodeIndex].LeftNode, min, median);
		BuildBVH(m_Nodes[NodeIndex].RightNode, median+1, max);
	}
	void BVHBuilder::BuildSAHBVH(int NodeIndex, int min, int max)
	{
		int nPrimitives = max - min;

		if (nPrimitives <= 36)
		{
			BVHNode leafNode;
			leafNode.BoundingBox = ComputeBounds(min, max);

			size_t k = 0;
			for (int i = min; i < max; i++)
			{
				leafNode.Primitive[k++] = i;
				leafNode.NumberOfPrims++;
			}

			leafNode.IsLeaf = true;
			m_Nodes.push_back(leafNode);
			m_Nodes[NodeIndex].RightNode = m_Nodes.size() - 1;

			return;
		}

		//AABB Bounds{};
		//Bounds = ComputeBounds(min, max);
		
		AABB CentroidBounds{};
		for (int i = min; i < max; i++)
			CentroidBounds.Grow(m_TriangleData[i].Centroid);

		int Dim = CentroidBounds.MaximumExtent();

		if (CentroidBounds.MaxValue[Dim] == CentroidBounds.MinValue[Dim])
		{
			BVHNode leafNode1;
			leafNode1.BoundingBox = ComputeBounds(min, max);

			size_t k = 0;
			for (int i = min; i < max; i++)
			{
				leafNode1.Primitive[k++] = i;
				leafNode1.NumberOfPrims++;
			}

			leafNode1.IsLeaf = true;
			m_Nodes.push_back(leafNode1);
			m_Nodes[NodeIndex].RightNode = m_Nodes.size() - 1;

			return;
		}
		
		

		constexpr int nBuckets = 12;

		BucketInfo Buckets[nBuckets];

		for (int i = min; i < max; i++)
		{
			int b = nBuckets * CentroidBounds.Offset(m_TriangleData[i].Centroid)[Dim];
			if (b == nBuckets) 
				b = nBuckets - 1;

			Buckets[b].count++;
			Buckets[b].bounds.Grow(m_TriangleData[i].Bounds());
		}

		float Cost[nBuckets - 1]{};
		
		for (int i = 0; i < nBuckets - 1; i++)
		{
			AABB b0, b1;
			int count0 = 0, count1 = 0;
			for (int j = 0; j <= i; j++) 
			{
				b0.Grow(Buckets[j].bounds);
				count0 += Buckets[j].count;
			}
			for (int j = i + 1; j < nBuckets; j++) 
			{
				b1.Grow(Buckets[j].bounds);
				count1 += Buckets[j].count;
			}
			Cost[i] = .125f + (count0 * b0.Area() +
				count1 * b1.Area()) / CentroidBounds.Area();
		}

		float MinCost = Cost[0];
		int MinCostBucket = 0;

		for (int i = 1; i < nBuckets - 1; i++)
		{
			if (Cost[i] < MinCost)
			{
				MinCost = Cost[i];
				MinCostBucket = i;
			}
		}
		

		auto pMid = std::partition(std::execution::par_unseq, m_TriangleData.begin() + min, m_TriangleData.begin() + max,
			[&](const Triangle& Other) 
			{
				int b = nBuckets * CentroidBounds.Offset(Other.Centroid)[Dim];
				if (b == nBuckets) b = nBuckets - 1;
				return b <= MinCostBucket;
			});

		int Mid = 0;

		if (pMid == m_TriangleData.begin() + max || pMid == m_TriangleData.begin() + min)
			Mid = (min + max) / 2;
		else
			Mid = pMid - m_TriangleData.begin();

		BVHNode LeftChild;
		LeftChild.BoundingBox = ComputeBounds(min, Mid);

		m_Nodes.push_back(LeftChild);
		m_Nodes[NodeIndex].LeftNode = m_Nodes.size() - 1;

		BVHNode RightChild;
		RightChild.BoundingBox = ComputeBounds(Mid, max);

		m_Nodes.push_back(RightChild);
		m_Nodes[NodeIndex].RightNode = m_Nodes.size() - 1;
		
		if (Mid - min > 0)
			BuildSAHBVH(m_Nodes[NodeIndex].LeftNode, min, Mid);
		
		
		if (max - Mid > 0)
			BuildSAHBVH(m_Nodes[NodeIndex].RightNode, Mid, max);
	}

	void BVHBuilder::BuildHLBVH()
	{
		AABB Bounds;
		for (size_t i = 0; i < m_TriangleData.size(); i++)
			Bounds.Grow(m_TriangleData[i].Centroid);

		ConcurrentVector<MortonPrimitive> MortonPrims(m_TriangleData.size());
		
		concurrency::parallel_for((size_t)0, MortonPrims.size(), (size_t)1, 
			[&](size_t i) 
			{
				constexpr int BitCount = 10;
				constexpr int Scale = 1 << BitCount;
				
				MortonPrims[i].TriangleIndex = i;
				
				glm::vec4 CentroidOffset = Bounds.Offset(m_TriangleData[i].Centroid);
				glm::vec3 Offset = glm::vec3(CentroidOffset) * (float)Scale;

				MortonPrims[i].MortonCode = EncodeMorton(Offset);
			});

		concurrency::parallel_radixsort(MortonPrims.begin(), MortonPrims.end(), 
			[](const MortonPrimitive& value)
			{
				return value.MortonCode;
			});

		ConcurrentVector<LBVHTreelet> BuildNodes;

		for (size_t start = 0, end = 1; end <= MortonPrims.size(); end++)
		{
			constexpr uint32_t mask = 0b00111111111111000000000000000000;
			/*
			* mask so that we only compare the bits highlighted with 1s
			*/
			
			if(end == MortonPrims.size() || 
				(MortonPrims[start].MortonCode & mask) != (MortonPrims[end].MortonCode & mask))
			{ 
				LBVHTreelet Treelet{};
				Treelet.min = start;
				Treelet.max = end;

				BuildNodes.push_back(Treelet);

				start = end;
			}
		}

		concurrency::parallel_for_each(BuildNodes.begin(), BuildNodes.end(), 
			[&](LBVHTreelet& node) 
			{
				constexpr int FirstBitIndex = 29 - 12;

				BVHNode LocalRoot;

				for (size_t i = node.min; i < node.max; i++)
					LocalRoot.BoundingBox.Grow(m_TriangleData[MortonPrims[i].TriangleIndex].Bounds());

				node.LocalNodes.push_back(LocalRoot);

				BuildHLBVHNode(node, MortonPrims, node.min, node.max, node.max - node.min, FirstBitIndex, 0);
			});

		ConcurrentVector<BVHTreeletNode> FinishedNodes;

		int i = 0;

		for (auto& Treelet : BuildNodes)
		{
			BVHTreeletNode TreeletNode;
			TreeletNode.Node = Treelet.LocalNodes[0];
			TreeletNode.TreeletIndex = i++;

			FinishedNodes.push_back(TreeletNode);
		}
		

		BVHNode RootNode;
		
		for (size_t i = 0; i < FinishedNodes.size(); i++)
			RootNode.BoundingBox.Grow(FinishedNodes[i].Node.BoundingBox);

		m_Nodes.push_back(RootNode);

		BuildUpperSAH(FinishedNodes, BuildNodes, 0, 0, FinishedNodes.size());
	}

	void BVHBuilder::BuildUpperSAH(
		ConcurrentVector<BVHTreeletNode>& Nodes, 
		ConcurrentVector<LBVHTreelet>& Treelets, 
		int NodeIndex, 
		int min, 
		int max)
	{
		int nNodes = max - min;

		if (nNodes == 1)
		{
			m_Nodes.push_back(Nodes[min].Node);
			m_Nodes[NodeIndex].LeftNode = m_Nodes.size() - 1;

			size_t StartIndex = m_Nodes[NodeIndex].LeftNode;

			auto& LocalNodes = Treelets[Nodes[min].TreeletIndex].LocalNodes;

			if (LocalNodes.size() > 1)
			{
				m_Nodes.insert(m_Nodes.end(), LocalNodes.begin() + 1, LocalNodes.end());

				for (size_t i = StartIndex; i < m_Nodes.size(); i++)
				{
					#if 0
					TGE_LOG_INFO(m_Nodes[i].RightNode, " ", m_Nodes[i].RightNode + StartIndex);
					TGE_LOG_INFO(LocalNodes[m_Nodes[i].RightNode].BoundingBox.MinValue.x, " ", 
						m_Nodes[m_Nodes[i].RightNode + StartIndex].BoundingBox.MinValue.x);
					#endif

					if (m_Nodes[i].RightNode != -1)
						m_Nodes[i].RightNode += StartIndex;

					if (m_Nodes[i].LeftNode != -1)
						m_Nodes[i].LeftNode += StartIndex;
				}
			}
				

			if (max >= Nodes.size() || max == min)
				return;

			m_Nodes.push_back(Nodes[max].Node);
			m_Nodes[NodeIndex].RightNode = m_Nodes.size() - 1;

			StartIndex = m_Nodes[NodeIndex].RightNode;

			LocalNodes = Treelets[Nodes[max].TreeletIndex].LocalNodes;

			if (LocalNodes.size() > 1)
			{
				m_Nodes.insert(m_Nodes.end(), LocalNodes.begin() + 1, LocalNodes.end());

				for (size_t i = StartIndex; i < m_Nodes.size(); i++)
				{
					if (m_Nodes[i].RightNode != -1)
						m_Nodes[i].RightNode += StartIndex;

					if (m_Nodes[i].LeftNode != -1)
						m_Nodes[i].LeftNode += StartIndex;
				}
			}
			

			return;
		}

		AABB Bounds;

		for (size_t i = min; i < max; i++)
			Bounds.Grow(Nodes[i].Node.BoundingBox);

		AABB CentroidBounds;
		for (size_t i = min; i < max; i++)
		{
			glm::vec4 Centroid = Nodes[i].Node.BoundingBox.MinValue + Nodes[i].Node.BoundingBox.MaxValue * 0.5f;
			CentroidBounds.Grow(Centroid);
		}

		int Dim = CentroidBounds.MaximumExtent();
		
		constexpr int nBuckets = 12;

		BucketInfo Buckets[nBuckets];

		for (int i = min; i < max; i++)
		{
			float Centroid = Nodes[i].Node.BoundingBox.MinValue[Dim] + Nodes[i].Node.BoundingBox.MaxValue[Dim] * 0.5f;

			int b = nBuckets * ((Centroid - CentroidBounds.MinValue[Dim]) / 
				(CentroidBounds.MaxValue[Dim] - CentroidBounds.MinValue[Dim]));

			if (b == nBuckets)
				b = nBuckets - 1;

			Buckets[b].count++;
			Buckets[b].bounds.Grow(Nodes[i].Node.BoundingBox);
		}

		float Cost[nBuckets - 1]{};

		for (int i = 0; i < nBuckets - 1; i++)
		{
			AABB b0, b1;
			int count0 = 0, count1 = 0;
			for (int j = 0; j <= i; j++)
			{
				b0.Grow(Buckets[j].bounds);
				count0 += Buckets[j].count;
			}
			for (int j = i + 1; j < nBuckets; j++)
			{
				b1.Grow(Buckets[j].bounds);
				count1 += Buckets[j].count;
			}
			Cost[i] = .125f + (count0 * b0.Area() + count1 * b1.Area()) / Bounds.Area();
		}

		float MinCost = Cost[0];
		int MinCostBucket = 0;

		for (int i = 1; i < nBuckets - 1; i++)
		{
			if (Cost[i] < MinCost)
			{
				MinCost = Cost[i];
				MinCostBucket = i;
			}
		}


		auto pMid = std::partition(Nodes.begin() + min, Nodes.begin() + max,
			[&](const BVHTreeletNode& Other)
			{
				float Centroid = Other.Node.BoundingBox.MinValue[Dim] + Other.Node.BoundingBox.MaxValue[Dim] * 0.5f;

				int b = nBuckets * ((Centroid - CentroidBounds.MinValue[Dim]) / 
					(CentroidBounds.MaxValue[Dim] - CentroidBounds.MinValue[Dim]));

				if (b == nBuckets) b = nBuckets - 1;

				return b <= MinCostBucket;
			});

		int Mid = 0;

		if (pMid == Nodes.begin() + max || pMid == Nodes.begin() + min)
			Mid = (min + max) / 2;
		else
			Mid = pMid - Nodes.begin();

		BVHNode LeftChild;

		for (size_t i = min; i < Mid; i++)
			LeftChild.BoundingBox.Grow(Nodes[i].Node.BoundingBox);

		m_Nodes.push_back(LeftChild);
		m_Nodes[NodeIndex].LeftNode = m_Nodes.size() - 1;

		BVHNode RightChild;
		
		for (size_t i = Mid; i < max; i++)
			RightChild.BoundingBox.Grow(Nodes[i].Node.BoundingBox);

		m_Nodes.push_back(RightChild);
		m_Nodes[NodeIndex].RightNode = m_Nodes.size() - 1;

		if (Mid - min > 0)
			BuildUpperSAH(Nodes, Treelets, m_Nodes[NodeIndex].LeftNode, min, Mid);


		if (max - Mid > 0)
			BuildUpperSAH(Nodes, Treelets, m_Nodes[NodeIndex].RightNode, Mid, max);
	}

	void BVHBuilder::BuildHLBVHNode(
		LBVHTreelet& NodeToBuild,
		const ConcurrentVector<MortonPrimitive>& Prims,
		int Start, int End, int nPrimitives, int BitIndex, int NodeIndex)
	{
		constexpr int MaxPrimInNode = 36;

		constexpr int DesiredPrimsInNode = 4;

		if (BitIndex == -1 || nPrimitives <= DesiredPrimsInNode)
		{
			if (nPrimitives >= MaxPrimInNode)
			{
				int Remaining = nPrimitives - MaxPrimInNode;

				BVHNode LeftLeafNode;
				LeftLeafNode.IsLeaf = true;
				LeftLeafNode.NumberOfPrims = MaxPrimInNode; // Use MaxPrimInNode instead of nPrimitives / 2

				for (size_t i = 0; i < MaxPrimInNode; i++)
				{
					LeftLeafNode.BoundingBox.Grow(m_TriangleData[Prims[Start + i].TriangleIndex].Bounds());
					LeftLeafNode.Primitive[i] = Prims[Start + i].TriangleIndex;
				}

				NodeToBuild.LocalNodes.push_back(LeftLeafNode);
				NodeToBuild.LocalNodes[NodeIndex].LeftNode = NodeToBuild.LocalNodes.size() - 1;

				BVHNode RightLeafNode;
				RightLeafNode.IsLeaf = true;
				RightLeafNode.NumberOfPrims = Remaining;

				int k = 0;

				for (size_t i = MaxPrimInNode; i < MaxPrimInNode + Remaining; i++)
				{
					RightLeafNode.BoundingBox.Grow(m_TriangleData[Prims[Start + i].TriangleIndex].Bounds());
					RightLeafNode.Primitive[k++] = Prims[Start + i].TriangleIndex;
				}

				NodeToBuild.LocalNodes.push_back(RightLeafNode);
				NodeToBuild.LocalNodes[NodeIndex].RightNode = NodeToBuild.LocalNodes.size() - 1;
			}
			else
			{
				BVHNode LeftLeafNode;
				LeftLeafNode.IsLeaf = true;
				LeftLeafNode.NumberOfPrims = nPrimitives; 

				for (size_t i = 0; i < nPrimitives; i++)
				{
					LeftLeafNode.BoundingBox.Grow(m_TriangleData[Prims[Start + i].TriangleIndex].Bounds());
					LeftLeafNode.Primitive[i] = Prims[Start + i].TriangleIndex;
				}

				NodeToBuild.LocalNodes.push_back(LeftLeafNode);
				NodeToBuild.LocalNodes[NodeIndex].LeftNode = NodeToBuild.LocalNodes.size() - 1;
			}

			return;
		}


		int mask = 1 << BitIndex;

		if ((Prims[Start].MortonCode & mask) == (Prims[End - 1].MortonCode & mask))
		{
			BuildHLBVHNode(NodeToBuild, Prims, Start, End, nPrimitives, BitIndex - 1, NodeIndex);
			return;
		}

		// Find split point for this dimension
		int SearchStart = Start, SearchEnd = End - 1;
		while (SearchStart + 1 != SearchEnd) 
		{
			int mid = (SearchStart + SearchEnd) / 2;
			if ((Prims[SearchStart].MortonCode & mask) == (Prims[mid].MortonCode & mask))
				SearchStart = mid;
			else 
				SearchEnd = mid;
		}

		int SplitOffset = SearchEnd;

		BVHNode LeftChild;
		BVHNode RightChild;
		
		for (size_t i = Start; i < SplitOffset; i++)
			LeftChild.BoundingBox.Grow(m_TriangleData[Prims[i].TriangleIndex].Bounds());

		for (size_t i = SplitOffset; i < End; i++)
			RightChild.BoundingBox.Grow(m_TriangleData[Prims[i].TriangleIndex].Bounds());


		NodeToBuild.LocalNodes.push_back(LeftChild);
		NodeToBuild.LocalNodes[NodeIndex].LeftNode = NodeToBuild.LocalNodes.size() - 1;

		BuildHLBVHNode(
			NodeToBuild,
			Prims,
			Start,
			SplitOffset,
			SplitOffset - Start,
			BitIndex - 1,
			NodeToBuild.LocalNodes[NodeIndex].LeftNode);

		NodeToBuild.LocalNodes.push_back(RightChild);
		NodeToBuild.LocalNodes[NodeIndex].RightNode = NodeToBuild.LocalNodes.size() - 1;

		BuildHLBVHNode(
			NodeToBuild,
			Prims,
			SplitOffset,
			End,
			End - SplitOffset,
			BitIndex - 1,
			NodeToBuild.LocalNodes[NodeIndex].RightNode);
	}

}
