#pragma once

#include "VertexArray.h"
#include "Texture.h"
#include "Buffers.h"
#include "ECS/BaseComponents.h"

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
		float  TextureIndex;
	};

	struct InstanceBufferData
	{
	public:
		std::vector<uint32_t> Indicies;
		std::vector<float> Data;

		bool operator==(const InstanceBufferData& other)
		{
			return Indicies == other.Indicies && Data == other.Data;
		}
	};

	class InstanceBuffer
	{
	public:
		InstanceBuffer(const InstanceBufferData& data);
		~InstanceBuffer();

		void PushData(const glm::mat4& Transform, const Ecs::MaterialComponent& Material);

		void BeginInstanceBatch();
		void EndInstanceBatch();

		inline const uint32_t GetNumberOfInstances() const { return m_InstanceCount; }
		inline const uint32_t GetCount() const { return m_Count; }

	private:
		const size_t m_SizeOfSlot = sizeof(glm::mat4) + sizeof(OpenGLMaterial);

		uint32_t m_InstanceCount = 0;
		uint32_t m_TextureBufferSize = 32;
		uint32_t m_CurrentTextureSlot = 0;
		uint32_t m_Count = 0;
		uint32_t m_InstanceBufferSize = m_SizeOfSlot * 10000;

		AttribBufferReturnData m_AttributeDataCopy;

		OpenGLVertexArray m_InstanceVAO;

		OpenGLBuffer m_InstanceVertexBuffer; //static data
		
		OpenGLBuffer m_InstanceBuffer; //per instance data (double buffering strategy
		OpenGLBuffer m_InstanceSecondBuffer;
		OpenGLBuffer m_InstanceThirdBuffer;

		uint32_t m_CurrentBufferIndex = 0;

		OpenGLBuffer m_InstanceIndexBuffer; //contains indicies
		OpenGLBuffer m_InstanceUniformTextureBuffer;

		std::unordered_map<GLuint64, size_t> m_TextureSlots; //gpu handle to index

		float* m_BufferMapBase = nullptr;
		GLuint64* m_TextureBufferBase = nullptr;
		
		ptrdiff_t m_MapOffset = 0;
	};

}


namespace TooGoodEngine{

	using InstanceID = std::size_t;
	
	
	struct RenderData
	{
		static constexpr InstanceID QuadInstanceID = 0;

		std::shared_ptr<Framebuffer> RenderFramebuffer;
		
		std::unique_ptr<Shader> InitialColorPass;
		
		std::shared_ptr<Texture> TestTexture;
		std::shared_ptr<Texture> ColorBuffer;
		std::shared_ptr<Texture> DepthBuffer;

		std::vector<InstanceBuffer> GroupedInstances;

		BaseCamera* ReferenceCamera = nullptr;
	};

	class Renderer
	{
	public:
		Renderer() = delete;
		~Renderer() = delete;
		
		static void Init();

		static void Begin(BaseCamera& RefCamera);

		static void DrawPrimitiveQuad(const glm::vec3& Pos);
		static void DrawPrimitiveQuad(const glm::mat4& Transform, const Ecs::MaterialComponent& Material);

		static void End();

		static void Shutdown();

		//TODO: remove when finished debugging
		static std::shared_ptr<Texture> GetColorBuffer() { return m_RenderData.ColorBuffer; }
		static std::shared_ptr<Texture> GetDepthBuffer() { return m_RenderData.DepthBuffer; }

	private:
		static RenderData m_RenderData;
	};


	

}

