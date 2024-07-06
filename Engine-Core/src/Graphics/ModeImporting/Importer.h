#pragma once

#include <filesystem>
#include <xhash>

#include <glm/common.hpp>

#include "Graphics/Texture.h"

namespace TooGoodEngine {

	struct Face
	{
		uint32_t a, b, c;

		bool operator==(const Face& other) const
		{
			return other.a == a && other.b == b && other.c == c;
		}
	};
#pragma pack(push, 1) // Pack the structure with byte alignment

	struct ModelVertex
	{
		ModelVertex(float oA, float oB, float oC) :a(oA), b(oB), c(oC) {}
		float a, b, c;

		bool operator==(const ModelVertex& other) const
		{
			return other.a == a && other.b == b && other.c == c;
		}
	};
	struct TexelCoord
	{
		TexelCoord(float oA, float oB) :a(oA), b(oB) {}
		float a, b;
	};

	struct SingleVertexInBuffer
	{
		SingleVertexInBuffer(const ModelVertex& oPosition, const ModelVertex& oNormal, const TexelCoord& oTexCoord) :
			Position(oPosition), Normal(oNormal), TexCoord(oTexCoord)
		{}

		ModelVertex Position;
		ModelVertex Normal;
		TexelCoord TexCoord;
	};

#pragma pack(pop) // Restore the default packing

	struct ModelData
	{
		std::vector<float> Verticies;
		std::vector<uint32_t> Indicies;
	};

	struct MeshMaterial
	{
		std::shared_ptr<Texture> Texture;

		glm::vec4 Albedo;
		glm::vec3 Reflectivity;
		glm::vec3 EmissionColor;
		float     EmissionPower;
		float	  Roughness;
		float     Metallic;
	};

	struct Mesh
	{
		std::vector<float> Vertices;
		std::vector<uint32_t> Indicies;
		MeshMaterial Material;
	};

	struct Model
	{
		std::vector<Mesh> MeshList;
	};

	class ModelImporter
	{
	public:
		ModelImporter() = default;
		virtual ~ModelImporter() = default;

		virtual ModelData Read(const std::filesystem::path& path) { return {}; };
		virtual Model	  ImportModel(const std::filesystem::path& path) { return {}; };
	};
}

namespace std {
	template <>
	struct hash<TooGoodEngine::Face> {
		size_t operator()(const TooGoodEngine::Face& obj) const {
			uint64_t result = 0x23F13452EE;
			result ^= obj.a + 0x9e3779b9 + (result << 5) + (result >> 2);
			result ^= obj.b + 0x9e3779b9 + (result << 5) + (result >> 2);
			result ^= obj.c + 0x9e3779b9 + (result << 5) + (result >> 2);
			return std::hash<uint64_t>()(result);
		}
	};

	template <>
	struct hash<TooGoodEngine::ModelVertex> {
		size_t operator()(const TooGoodEngine::ModelVertex& obj) const {
			return std::hash<float>()(obj.a + obj.b + obj.c);
		}
	};
}