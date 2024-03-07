#pragma once

#include "VertexArray.h"
#include "Texture.h"
#include "Buffers.h"
#include "ECS/BaseComponents.h"
#include "ModeImporting/Importer.h"
#include "RayTracing/BVHBuilder.h"

#include <unordered_map>
#include <xhash>
#include <memory>
#include <vector>
#include <set>




namespace TooGoodEngine {

	/*
	*			 positions			     normals		Texture Coordinates
	*  data = {0.0f, 0.0f, 0.0f,	1.0f, 0.0f, 0.0f,    1.0f, 1.0f}
	*  data must be in this format
	*  all instance buffer primitve type will be triangles
	*  support for lines will be seperate in a specialized debug renderer
	*/


	//GPU side material
	struct OpenGLMaterial
	{
		glm::vec4 Albedo;
		glm::vec3 Reflectivity;
		float	  Metallic;
		glm::vec3 Emission; //color and power
		float	  Roughness;
		float     TextureIndex;
	};

	//GPU side directional lighting information
	struct DirectionalLightSource
	{
		glm::vec4 Direction;
		glm::vec3 Color;
		float Intensity;
	};

	struct MeshInstanceBufferData
	{
	public:
		std::vector<float> Data;
		std::vector<uint32_t> Indicies;
		MeshMaterial DefaultMaterial;

		bool operator==(const MeshInstanceBufferData& other)
		{
			return Indicies == other.Indicies && Data == other.Data;
		}
	};

	class MeshInstanceBuffer
	{
	public:
		MeshInstanceBuffer(const Mesh& data);
		~MeshInstanceBuffer();

		void PushData(const glm::mat4& Transform, const Ecs::MaterialComponent& Material);
		void PushData(const glm::mat4& Transform);

		void BeginInstanceBatch();
		void EndInstanceBatch();
		
		inline const Mesh& GetCopy() const { return m_MeshCopy; }
		inline const uint32_t GetNumberOfInstances() const { return m_InstanceCount; }
		inline const uint32_t GetCount() const { return m_Count; }

	private:
		const size_t m_SizeOfSlot = sizeof(glm::mat4) + sizeof(OpenGLMaterial);

		Mesh m_MeshCopy;
		
		std::shared_ptr<Texture> m_DefaultTexture;
		OpenGLMaterial m_DefaultMaterial;

		uint32_t m_InstanceCount = 0;
		uint32_t m_CurrentTextureSlot = 1;
		uint32_t m_Count = 0;
		uint32_t m_MeshInstanceBufferSize = m_SizeOfSlot * 10;
		uint64_t m_TextureBufferSize = 32 * sizeof(uint64_t);


		AttribBufferReturnData m_AttributeDataCopy;

		OpenGLVertexArray m_InstanceVAO;

		OpenGLBuffer m_InstanceVertexBuffer; //static data
		
		OpenGLBuffer m_MeshInstanceBuffer; //per instance data (double buffering strategy
		OpenGLBuffer m_InstanceSecondBuffer;
		OpenGLBuffer m_InstanceThirdBuffer;

		uint32_t m_CurrentBufferIndex = 0;

		OpenGLBuffer m_InstanceIndexBuffer; //contains indicies
		OpenGLBuffer m_InstanceTextureBuffer;

		std::unordered_map<GLuint64, size_t> m_TextureSlots; //gpu handle to index

		float* m_BufferMapBase = nullptr;
		GLuint64* m_TextureBufferBase = nullptr;
		
		ptrdiff_t m_MapOffset = 0;
	};

	struct ModelInstanceBuffer
	{
		std::vector<MeshInstanceBuffer> InstanceMeshs;
	};
}


namespace TooGoodEngine{

	using InstanceID = std::size_t;
	
	
	struct RenderData
	{
		static constexpr InstanceID QuadInstanceID = 0;
		float Scale = 4.0f;

		uint32_t FramebufferWidth = 0, FramebufferHeight = 0;
		uint32_t DownScaledWidth = 0, DownScaledHeight = 0;

		std::unique_ptr<BVHBuilder> BoundingVolumeHierarchy;
		BuildType BoundingBuildType = BuildType::HLSplit;

		std::shared_ptr<Texture> FinalImage;
		std::shared_ptr<Framebuffer> FinalFramebuffer;

		std::shared_ptr<Texture> AccumulationBuffer;

		std::shared_ptr<Texture> ResizedColorbuffer;
		std::shared_ptr<Texture> ResizedDepthBuffer;
		std::shared_ptr<Texture> ResizedNormalBuffer;
		std::shared_ptr<Texture> ResizedReflectiveAndMetallic;
		std::shared_ptr<Texture> ResizedEmissionAndRoughness;


		std::shared_ptr<Texture> DownSampledColorbuffer;
		std::shared_ptr<Texture> DownSampledDepthBuffer;
		std::shared_ptr<Texture> DownSampledNormal;
		std::shared_ptr<Texture> DownSampledReflectiveAndMetallic;
		std::shared_ptr<Texture> DownSampledEmissionAndRoughness;

		std::shared_ptr<Texture> ShadowMap;

		std::shared_ptr<Framebuffer> RenderFramebuffer;
		std::shared_ptr<Framebuffer> ResizedFramebuffer;
		std::shared_ptr<Framebuffer> DownSampledFramebuffer;

		std::unique_ptr<Shader> GBufferPass;
		std::unique_ptr<Shader> ShadowPass;
		std::unique_ptr<Shader> DirectLightingPass;
		std::unique_ptr<Shader> GlobalIlluminationPass;
		
		std::shared_ptr<Texture> TestTexture;

		std::shared_ptr<Texture> NormalBuffer;
		std::shared_ptr<Texture> ColorBuffer;
		std::shared_ptr<Texture> ReflectiveAndMetallic;
		std::shared_ptr<Texture> EmissionAndRoughnessBuffer;

		std::shared_ptr<Texture> DepthBuffer;

		std::vector<MeshInstanceBuffer> GroupedMeshInstances;
		std::vector<ModelInstanceBuffer> GroupedModelInstances;

		BaseCamera* ReferenceCamera = nullptr;
		glm::vec3 CurrentFront = glm::vec3(0.0, 0.0, 0.0);
		glm::vec3 CurrentPosition = glm::vec3(0.0, 0.0, 0.0);

		std::vector<DirectionalLightSource> DirectionalLightSources;
		std::unique_ptr<OpenGLBuffer> DirectionalLightBuffer;

		size_t CurrentLightBufferCapacity = 10;

		int FrameIndex = 1;
	};

	class Renderer
	{
	public:
		Renderer() = delete;
		~Renderer() = delete;
		
		static void Init();

		static _NODISCARD InstanceID AddUniqueMesh(const Mesh& data);
		static _NODISCARD InstanceID AddUniqueModel(const Model& data);

		static void Begin(BaseCamera& RefCamera);

		static void DrawMeshInstance(InstanceID id, const glm::mat4& Transform, const Ecs::MaterialComponent& Material);
		
		static void DrawModelInstance(InstanceID id, const glm::mat4& Transforms, const Ecs::MaterialComponent& Material);
		static void DrawModelInstance(InstanceID id, const glm::mat4& Transforms);

		static void DrawPrimitiveQuad(const glm::vec3& Pos);
		static void DrawPrimitiveQuad(const glm::mat4& Transform, const Ecs::MaterialComponent& Material);
		
		static void ChangeScaledResolution(float NewScale);
		static void ChangeMultiSampleRate(int NewRate);

		static void ChangeBVHBuildType(BuildType NewBuildType);

		static void AddDirectionalLight(const DirectionalLightSource& Src);

		static void End();

		static void Shutdown();

		//TODO: remove when finished debugging
		static std::shared_ptr<Texture> GetColorBuffer() { return m_RenderData.FinalImage; }
		static std::shared_ptr<Texture> GetDepthBuffer() { return m_RenderData.DepthBuffer; }

	private:
		static RenderData m_RenderData;
	};
}

