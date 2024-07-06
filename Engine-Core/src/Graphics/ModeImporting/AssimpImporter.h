#pragma once

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags


#include <concurrent_vector.h>
#include <ppl.h>

#include "Importer.h"


namespace TooGoodEngine {

	class AssimpImporter : public ModelImporter
	{
	public:
		AssimpImporter() = default;
		virtual ~AssimpImporter() = default;

		virtual ModelData Read(const std::filesystem::path& path) override { return {}; }
		virtual Model	  ImportModel(const std::filesystem::path& path) override;
	private:
		concurrency::concurrent_vector<Mesh> ModelMeshs;

		void ProcessNode(const aiNode* node, const aiScene* scene);
		void ProcessMesh(const aiMesh* mesh, const aiScene* scene);
	};

}