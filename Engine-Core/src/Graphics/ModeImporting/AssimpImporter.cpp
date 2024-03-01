#include "pch.h"
#include "AssimpImporter.h"

namespace TooGoodEngine {
	Model AssimpImporter::ImportModel(const std::filesystem::path& path)
	{
		ModelMeshs.clear();

		Assimp::Importer aImporter;

		const aiScene* scene = aImporter.ReadFile(path.string(), aiProcess_Triangulate      |
																 aiProcess_FlipUVs	        |
																 aiProcess_GenSmoothNormals | 
																 aiProcess_JoinIdenticalVertices 
																 );

		TGE_FORCE_ASSERT(scene, aImporter.GetErrorString());

		ProcessNode(scene->mRootNode, scene);

		std::vector<Mesh> Data(ModelMeshs.begin(), ModelMeshs.end());

		return { Data };
	}

	void AssimpImporter::ProcessNode(const aiNode* node, const aiScene* scene)
	{
		for (size_t i = 0; i < node->mNumChildren; i++)
			ProcessNode(node->mChildren[i], scene);

		concurrency::parallel_for(0u, (uint32_t)node->mNumMeshes, 1u, 
			[&](uint32_t i) 
			{
				ProcessMesh(scene->mMeshes[node->mMeshes[i]], scene);
			});
		
	}
	void AssimpImporter::ProcessMesh(const aiMesh* mesh, const aiScene* scene)
	{
		std::vector<aiVector3D> aiVertices;
		std::vector<aiVector3D> aiNormals;
		std::vector<aiVector3D> aiTextureCoordinates;

		std::vector<uint32_t> Indices;

		for (size_t i = 0; i < mesh->mNumVertices; i++)
			aiVertices.push_back(mesh->mVertices[i]);

		for (size_t i = 0; i < mesh->mNumVertices; i++)
			aiNormals.push_back(mesh->mNormals[i]);


		if (!(mesh->mTextureCoords[0] == nullptr))
		{
			for (size_t i = 0; i < mesh->mNumVertices; i++)
				aiTextureCoordinates.push_back(mesh->mTextureCoords[0][i]);
		}
		else
		{
			for (size_t i = 0; i < mesh->mNumVertices; i++)
				aiTextureCoordinates.push_back(aiVector3D(0.0, 0.0, 0.0));
		}
		

		for (size_t i = 0; i < mesh->mNumFaces; i++)
		{
			for (size_t j = 0; j < mesh->mFaces[i].mNumIndices; j++)
				Indices.push_back(mesh->mFaces[i].mIndices[j]);
		}

		/*
		* OpenGL needs the format of this data in the form of
		*		Vertex	  Normal   TexCoords
		* 
		*/

		Mesh ModelMesh;

		ModelMesh.Indicies = Indices;

		/*
		* 3 floats for position, 3 floats for normal, and 2 floats for texture coordinate
		*/
		
		for (size_t i = 0; i < aiVertices.size(); i++)
		{
			ModelMesh.Vertices.push_back(aiVertices[i].x);
			ModelMesh.Vertices.push_back(aiVertices[i].y);
			ModelMesh.Vertices.push_back(aiVertices[i].z);

			ModelMesh.Vertices.push_back(aiNormals[i].x);
			ModelMesh.Vertices.push_back(aiNormals[i].y);
			ModelMesh.Vertices.push_back(aiNormals[i].z);

			ModelMesh.Vertices.push_back(aiTextureCoordinates[i].x);
			ModelMesh.Vertices.push_back(aiTextureCoordinates[i].y);
		}

		ModelMesh.Material = {}; 

		const aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];

		aiColor4D albedo;
		aiColor3D reflectivity;
		aiColor3D emissionColor;
		ai_real emissionPower = 0.0f;
		ai_real roughness = 0.0f;
		ai_real metallic = 0.0f;


		aiMat->Get(AI_MATKEY_COLOR_DIFFUSE,		 albedo);
		aiMat->Get(AI_MATKEY_COLOR_REFLECTIVE,   reflectivity);
		aiMat->Get(AI_MATKEY_COLOR_EMISSIVE,	 emissionColor);
		aiMat->Get(AI_MATKEY_EMISSIVE_INTENSITY, emissionPower);
		aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR,	 roughness);
		aiMat->Get(AI_MATKEY_METALLIC_FACTOR,	 metallic);

		memcpy(&ModelMesh.Material.Albedo,		  &albedo,        sizeof(float) * 4);
		memcpy(&ModelMesh.Material.Reflectivity,  &reflectivity,  sizeof(float) * 3);
		memcpy(&ModelMesh.Material.EmissionColor, &emissionColor, sizeof(float) * 3);
		memcpy(&ModelMesh.Material.EmissionPower, &emissionPower, sizeof(float));
		memcpy(&ModelMesh.Material.Roughness,	  &roughness,	  sizeof(float));
		memcpy(&ModelMesh.Material.Metallic,	  &metallic,	  sizeof(float));

		TextureData texData;
		texData.InternalFormat = TextureFormat::RGBA8;
		texData.Level = 1;

		aiString TexturePath;

		ModelMesh.Material.Texture = nullptr;

		if (aiMat->GetTexture(aiTextureType_BASE_COLOR, 0, &TexturePath) == AI_SUCCESS)
		{
			std::filesystem::path p = TexturePath.C_Str();
			std::filesystem::path Ext = p.extension();
			if (Ext == ".png")
				ModelMesh.Material.Texture = std::make_shared<Texture>(std::string_view(TexturePath.C_Str()), texData, Format::RGBA);
		}

		ModelMeshs.push_back(ModelMesh);
	}
}