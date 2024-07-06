#include "pch.h"
#include "Renderer.h"

#include "Utils/Clock.h"

#include <utility>

namespace TooGoodEngine {

	static std::vector<uint32_t> QuadIndicies =
	{
		0,1,2,2,3,0
	};

	static std::vector<float> QuadVerticies =
	{	//vertex				normal				//texture coordinates
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f, 1.0f,	0.0f, 0.0f,
		 0.5f, -0.5f, 0.0f,		0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f,  0.5f, 0.0f,		0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
		 -0.5f, 0.5f, 0.0f,		0.0f, 0.0f, 1.0f,   0.0f, 1.0f
	};

	MeshInstanceBuffer::MeshInstanceBuffer(const Mesh& data)
		: m_InstanceVAO(),
		m_InstanceVertexBuffer(BufferType::VertexBuffer, { data.Vertices.data(),  data.Vertices.size() * sizeof(GLfloat), GL_STATIC_DRAW}),
		m_MeshInstanceBuffer(BufferType::VertexBuffer, { nullptr, m_MeshInstanceBufferSize, GL_DYNAMIC_DRAW }),
		m_InstanceSecondBuffer(BufferType::VertexBuffer, { nullptr, m_MeshInstanceBufferSize, GL_DYNAMIC_DRAW}),
		m_InstanceThirdBuffer(BufferType::VertexBuffer, { nullptr, m_MeshInstanceBufferSize, GL_DYNAMIC_DRAW }),
		m_InstanceIndexBuffer(BufferType::IndexBuffer, { data.Indicies.data(), data.Indicies.size() * sizeof(GLuint), GL_STATIC_DRAW}),
		m_InstanceTextureBuffer(BufferType::ShaderStorageBuffer, {nullptr, m_TextureBufferSize, GL_DYNAMIC_DRAW }),
		m_Count((uint32_t)data.Indicies.size()), m_DefaultTexture(data.Material.Texture), m_MeshCopy(data)
	{
		TooGoodEngine::TextureData TextureData;
		TextureData.InternalFormat = TooGoodEngine::TextureFormat::RGBA32F;
		TextureData.Width = 1;
		TextureData.Height = 1;

		glm::vec4* TempData = new glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

		if (!m_DefaultTexture)
			m_DefaultTexture = std::make_shared<Texture>(&TempData->x, TextureData);

		delete TempData;
		
		m_DefaultMaterial.Albedo			= data.Material.Albedo;
		m_DefaultMaterial.Emission			= data.Material.EmissionColor * data.Material.EmissionPower;
		m_DefaultMaterial.Metallic			= data.Material.Metallic;
		m_DefaultMaterial.Reflectivity		= data.Material.Reflectivity;
		m_DefaultMaterial.Roughness			= data.Material.Roughness;
		m_DefaultMaterial.TextureIndex		= 0.0f;


		std::vector<AttributeType> InstanceVertexAttribs =
		{
			AttributeType::FLOAT_3, //vertex
			AttributeType::FLOAT_3, //normal
			AttributeType::FLOAT_2  //texture coordinate
		};

		std::vector<AttributeType> MeshInstanceBufferAttribs =
		{
			AttributeType::FLOAT_4, // albedo
			AttributeType::FLOAT_3, // reflectivity
			AttributeType::FLOAT,   // mettalic
			AttributeType::FLOAT_3, // emission
			AttributeType::FLOAT,   // roughness
			AttributeType::FLOAT,    // texture slot index
			AttributeType::MAT_4   // Transform
		};

		std::vector<BufferVertexAttributes> AttribData = 
		{
			{&m_InstanceVertexBuffer, InstanceVertexAttribs, false},
			{&m_MeshInstanceBuffer, MeshInstanceBufferAttribs, true} //true meaning is instanced
		};

		m_AttributeDataCopy = m_InstanceVAO.AttachAttribToBuffer(AttribData);

		m_InstanceVAO.AttachIndexBuffer(m_InstanceIndexBuffer);

		m_BufferMapBase = (float*)m_MeshInstanceBuffer.MapBufferRange();
		m_TextureBufferBase = (GLuint64*)m_InstanceTextureBuffer.MapBufferRange();

		m_TextureSlots[m_DefaultTexture->GetGPUHandle()] = 0;
		m_TextureBufferBase[0] = m_DefaultTexture->GetGPUHandle();
	}

	MeshInstanceBuffer::~MeshInstanceBuffer()
	{
		if (m_CurrentBufferIndex == 0)
			m_MeshInstanceBuffer.UnMap();
		else if (m_CurrentBufferIndex == 1)
			m_InstanceSecondBuffer.UnMap();
		else
			m_InstanceThirdBuffer.UnMap();
	}
	

	void MeshInstanceBuffer::PushData(const glm::mat4& Transform, const Ecs::MaterialComponent& Material)
	{	
		OpenGLMaterial Mat{};
		Mat.Albedo = Material.Albedo;
		Mat.Emission = Material.EmissionColor * Material.EmissionPower;
		Mat.Metallic = Material.Metallic;
		Mat.Reflectivity = Material.Reflectivity;
		Mat.Roughness = Material.Roughness;
		
		
		GLuint64 GPUHandle = 0;

		if (Material.MaterialTexture)
		{
			GPUHandle = Material.MaterialTexture->GetGPUHandle();

			if (m_TextureSlots.contains(GPUHandle))
				Mat.TextureIndex = (float)m_TextureSlots[GPUHandle];
			else
			{
				if (m_TextureBufferSize <= m_TextureSlots.size() * sizeof(uint64_t))
				{
					m_InstanceTextureBuffer.UnMap();

					m_TextureBufferSize = sizeof(GLuint64) * (((uint32_t)m_TextureSlots.size() + 1) * 2);
					m_InstanceTextureBuffer.Resize(m_TextureBufferSize);

					m_TextureBufferBase = (GLuint64*)m_InstanceTextureBuffer.MapBufferRange();
				}

				m_TextureSlots[GPUHandle] = m_CurrentTextureSlot;
				Mat.TextureIndex = (float)m_CurrentTextureSlot;
				m_TextureBufferBase[m_CurrentTextureSlot++] = GPUHandle;
	
			}
		}
		else
		{
			GPUHandle = m_DefaultTexture->GetGPUHandle();

			if (m_TextureSlots.contains(GPUHandle))
				Mat.TextureIndex = (float)m_TextureSlots[GPUHandle];
			else
			{
				if (m_TextureBufferSize <= m_TextureSlots.size() * sizeof(uint64_t))
				{
					m_InstanceTextureBuffer.UnMap();

					m_TextureBufferSize = sizeof(GLuint64) * (((uint32_t)m_TextureSlots.size() + 1) * 2);
					m_InstanceTextureBuffer.Resize(m_TextureBufferSize);

					m_TextureBufferBase = (GLuint64*)m_InstanceTextureBuffer.MapBufferRange();
				}

				m_TextureSlots[GPUHandle] = m_CurrentTextureSlot;
				Mat.TextureIndex = (float)m_CurrentTextureSlot;
				m_TextureBufferBase[m_CurrentTextureSlot++] = GPUHandle;

			}
		}


		const float* MatPtr = glm::value_ptr(Mat.Albedo);
		int size = sizeof(OpenGLMaterial) / sizeof(Mat.Albedo.x);
		memcpy(m_BufferMapBase + m_MapOffset, MatPtr, (size_t)size * sizeof(float));
		m_MapOffset += size;

		const float* Source = (const float*)glm::value_ptr(Transform);
		memcpy(m_BufferMapBase + m_MapOffset, Source, 16 * sizeof(float));
		m_MapOffset += 16;

		const size_t SizeOfMap = m_MapOffset * sizeof(float);

		if (SizeOfMap >= m_MeshInstanceBufferSize)
		{
			if (m_CurrentBufferIndex == 0)
				m_MeshInstanceBuffer.UnMap();
			else if (m_CurrentBufferIndex == 1)
				m_InstanceSecondBuffer.UnMap();
			else
				m_InstanceThirdBuffer.UnMap();

			m_MeshInstanceBufferSize = (uint32_t)SizeOfMap * 2;
			
			m_MeshInstanceBuffer.Resize(m_MeshInstanceBufferSize);
			m_InstanceSecondBuffer.Resize(m_MeshInstanceBufferSize);
			m_InstanceThirdBuffer.Resize(m_MeshInstanceBufferSize);

			if (m_CurrentBufferIndex == 0)
			{
				m_BufferMapBase = (float*)m_MeshInstanceBuffer.MapBufferRange();
				m_InstanceVAO.AttachVertexBuffer(m_MeshInstanceBuffer, 1, m_AttributeDataCopy.Offsets[1], m_AttributeDataCopy.Strides[1]);

			}
			else if (m_CurrentBufferIndex == 1)
			{
				m_BufferMapBase = (float*)m_InstanceSecondBuffer.MapBufferRange();
				m_InstanceVAO.AttachVertexBuffer(m_InstanceSecondBuffer, 1, m_AttributeDataCopy.Offsets[1], m_AttributeDataCopy.Strides[1]);
			}
			else
			{
				m_BufferMapBase = (float*)m_InstanceThirdBuffer.MapBufferRange();
				m_InstanceVAO.AttachVertexBuffer(m_InstanceThirdBuffer, 1, m_AttributeDataCopy.Offsets[1], m_AttributeDataCopy.Strides[1]);
			}
			
		}

		m_InstanceCount++;
	}

	void MeshInstanceBuffer::PushData(const glm::mat4& Transform)
	{
		const float* MatPtr = glm::value_ptr(m_DefaultMaterial.Albedo);
		int size = sizeof(OpenGLMaterial) / sizeof(m_DefaultMaterial.Albedo.x);
		memcpy(m_BufferMapBase + m_MapOffset, MatPtr, (size_t)size * sizeof(float));
		m_MapOffset += size;

		const float* Source = (const float*)glm::value_ptr(Transform);
		memcpy(m_BufferMapBase + m_MapOffset, Source, 16 * sizeof(float));
		m_MapOffset += 16;

		const size_t SizeOfMap = m_MapOffset * sizeof(float);

		if (SizeOfMap >= m_MeshInstanceBufferSize)
		{
			if (m_CurrentBufferIndex == 0)
				m_MeshInstanceBuffer.UnMap();
			else if (m_CurrentBufferIndex == 1)
				m_InstanceSecondBuffer.UnMap();
			else
				m_InstanceThirdBuffer.UnMap();

			m_MeshInstanceBufferSize = (uint32_t)SizeOfMap * 2;

			m_MeshInstanceBuffer.Resize(m_MeshInstanceBufferSize);
			m_InstanceSecondBuffer.Resize(m_MeshInstanceBufferSize);
			m_InstanceThirdBuffer.Resize(m_MeshInstanceBufferSize);

			if (m_CurrentBufferIndex == 0)
			{
				m_BufferMapBase = (float*)m_MeshInstanceBuffer.MapBufferRange();
				m_InstanceVAO.AttachVertexBuffer(m_MeshInstanceBuffer, 1, m_AttributeDataCopy.Offsets[1], m_AttributeDataCopy.Strides[1]);

			}
			else if (m_CurrentBufferIndex == 1)
			{
				m_BufferMapBase = (float*)m_InstanceSecondBuffer.MapBufferRange();
				m_InstanceVAO.AttachVertexBuffer(m_InstanceSecondBuffer, 1, m_AttributeDataCopy.Offsets[1], m_AttributeDataCopy.Strides[1]);
			}
			else
			{
				m_BufferMapBase = (float*)m_InstanceThirdBuffer.MapBufferRange();
				m_InstanceVAO.AttachVertexBuffer(m_InstanceThirdBuffer, 1, m_AttributeDataCopy.Offsets[1], m_AttributeDataCopy.Strides[1]);
			}

		}
		m_InstanceCount++;
	}

	void MeshInstanceBuffer::BeginInstanceBatch()
	{
		if (m_CurrentBufferIndex == 0)
			m_MeshInstanceBuffer.UnMap();
		else if (m_CurrentBufferIndex == 1)
			m_InstanceSecondBuffer.UnMap();
		else
			m_InstanceThirdBuffer.UnMap();

		m_InstanceTextureBuffer.BindBase(0);

		for (auto& [handle, ID] : m_TextureSlots)
			glMakeTextureHandleResidentARB(handle);

		m_InstanceTextureBuffer.UnMap();

		m_InstanceVAO.Bind();

	}

	void MeshInstanceBuffer::EndInstanceBatch()
	{
	 	for (auto& [handle, ID] : m_TextureSlots)
			glMakeTextureHandleNonResidentARB(handle);

		m_InstanceCount = 0;
		m_CurrentTextureSlot = 1;
		m_MapOffset = 0;

		m_TextureSlots.clear();
		m_TextureSlots[m_DefaultTexture->GetGPUHandle()] = 0;

		m_TextureBufferBase = (GLuint64*)m_InstanceTextureBuffer.MapBufferRange();
		m_TextureBufferBase[0] = m_DefaultTexture->GetGPUHandle();

		if (m_CurrentBufferIndex == 0)
		{
			m_BufferMapBase = (float*)m_InstanceSecondBuffer.MapBufferRange();
			m_InstanceVAO.AttachVertexBuffer(m_InstanceSecondBuffer, 1, m_AttributeDataCopy.Offsets[1], m_AttributeDataCopy.Strides[1]);
			m_CurrentBufferIndex = 1;
		}
		else if (m_CurrentBufferIndex == 1)
		{
			m_BufferMapBase = (float*)m_InstanceThirdBuffer.MapBufferRange();
			m_InstanceVAO.AttachVertexBuffer(m_InstanceThirdBuffer, 1, m_AttributeDataCopy.Offsets[1], m_AttributeDataCopy.Strides[1]);
			m_CurrentBufferIndex = 2;
		}
		else
		{
			m_BufferMapBase = (float*)m_MeshInstanceBuffer.MapBufferRange();
			m_InstanceVAO.AttachVertexBuffer(m_MeshInstanceBuffer, 1, m_AttributeDataCopy.Offsets[1], m_AttributeDataCopy.Strides[1]);
			m_CurrentBufferIndex = 0;
		}
	}


	RenderData Renderer::m_RenderData;

	void Renderer::Init()
	{
		BufferData BfData{};
		BfData.data = nullptr;
		BfData.DrawType = GL_DYNAMIC_DRAW;
		BfData.VertexSize = sizeof(DirectionalLightSource) * 10;

		m_RenderData.DirectionalLightBuffer = std::make_unique<OpenGLBuffer>(BufferType::ShaderStorageBuffer, BfData);

		BfData.VertexSize = sizeof(OpenGLMaterial) * 10;

		m_RenderData.MaterialBuffer = std::make_unique<OpenGLBuffer>(BufferType::ShaderStorageBuffer, BfData);

		BfData.VertexSize = sizeof(GLuint64) * 10;

		m_RenderData.TextureBuffer = std::make_unique<OpenGLBuffer>(BufferType::ShaderStorageBuffer, BfData);

		float Width = (float)Application::GetMainWindow().GetWidth();
		float Height = (float)Application::GetMainWindow().GetHeight();

		m_RenderData.FramebufferWidth = (uint32_t)Width;
		m_RenderData.FramebufferHeight = (uint32_t)Height;
		
		m_RenderData.DownScaledWidth = (uint32_t)(Width / m_RenderData.Scale);
		m_RenderData.DownScaledHeight = (uint32_t)(Height / m_RenderData.Scale);

		CreateTextures();
		CreateFramebuffers();

		{
			std::map<GLenum, std::string_view> ShaderList;
			ShaderList[GL_VERTEX_SHADER] = "../Engine-Core/src/Graphics/Shaders/RendererShaders/ColorPassVertex.glsl";
			ShaderList[GL_FRAGMENT_SHADER] = "../Engine-Core/src/Graphics/Shaders/RendererShaders/ColorPassFragment.glsl";

			m_RenderData.GBufferPass = Shader::Generate(ShaderList);
		}

		{
			std::map<GLenum, std::string_view> ShaderList;
			ShaderList[GL_COMPUTE_SHADER] = "../Engine-Core/src/Graphics/Shaders/RendererShaders/DirectLighting.glsl";

			m_RenderData.DirectLightingPass = Shader::Generate(ShaderList);
		}

		/*
		* Instance ID 0 will correspond to a primitive quad instance
		*/

		{
			Mesh InstanceData;

			InstanceData.Vertices = QuadVerticies;
			InstanceData.Indicies = QuadIndicies;
			InstanceData.Material = {};

			InstanceData.Material.Albedo = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			InstanceData.Material.EmissionColor = glm::vec3(1.0f, 1.0f, 1.0f);
			InstanceData.Material.Reflectivity = glm::vec3(1.0f, 1.0f, 1.0f);
			InstanceData.Material.EmissionPower = 1.0f;
			InstanceData.Material.Metallic = 0.0f;
			InstanceData.Material.Roughness = 0.0f;


			if (m_RenderData.GroupedMeshInstances.size() <= 0)
			{
				m_RenderData.GroupedMeshInstances.reserve(30);
				m_RenderData.GroupedModelInstances.reserve(30);
			}

			m_RenderData.GroupedMeshInstances.emplace_back(InstanceData);
		}

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		glEnable(GL_DEPTH_TEST);



		
	}
	void Renderer::Begin(BaseCamera& RefCamera)
	{
		m_RenderData.ReferenceCamera = &RefCamera;

		if (m_RenderData.CurrentPosition != m_RenderData.ReferenceCamera->GetPosition() ||
			m_RenderData.CurrentFront != m_RenderData.ReferenceCamera->GetFront())
		{
			m_RenderData.FrameIndex = 1;

			m_RenderData.CurrentPosition = m_RenderData.ReferenceCamera->GetPosition();
			m_RenderData.CurrentFront = m_RenderData.ReferenceCamera->GetFront();
		}

	}
	void Renderer::End()
	{
		glm::mat4 Projection = m_RenderData.ReferenceCamera->GetProjection();
		glm::mat4 View = m_RenderData.ReferenceCamera->GetView();

		glm::mat4 ViewProjectionMatrix = Projection * View;

		m_RenderData.RenderFramebuffer->Bind();

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		glViewport(0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		
		
		for (auto& InstanceMeshBatch : m_RenderData.GroupedMeshInstances)
		{
			if (InstanceMeshBatch.GetNumberOfInstances() == 0)
				continue;

			m_RenderData.GBufferPass->Use();
			m_RenderData.GBufferPass->SetUniformMat4("ViewProjection", ViewProjectionMatrix);

			InstanceMeshBatch.BeginInstanceBatch();

			glDrawElementsInstanced(
				GL_TRIANGLES,                     
				(GLuint)(InstanceMeshBatch.GetCount()),
				GL_UNSIGNED_INT,                   
				nullptr,                          
				(GLuint)(InstanceMeshBatch.GetNumberOfInstances())
			);

			InstanceMeshBatch.EndInstanceBatch();
		}

		for (auto& InstanceModelBatch : m_RenderData.GroupedModelInstances)
		{
			for (auto& InstanceMeshBatch : InstanceModelBatch.InstanceMeshs)
			{
				if (InstanceMeshBatch.GetNumberOfInstances() == 0)
					continue;

				m_RenderData.GBufferPass->Use();
				m_RenderData.GBufferPass->SetUniformMat4("ViewProjection", ViewProjectionMatrix);

				InstanceMeshBatch.BeginInstanceBatch();

				glDrawElementsInstanced(
					GL_TRIANGLES,
					(GLuint)(InstanceMeshBatch.GetCount()),
					GL_UNSIGNED_INT,
					nullptr,
					(GLuint)(InstanceMeshBatch.GetNumberOfInstances())
				);

				InstanceMeshBatch.EndInstanceBatch();
			}
		}


		m_RenderData.RenderFramebuffer->UnBind();

		void* LightData = m_RenderData.DirectionalLightBuffer->Map();
		memcpy(LightData, m_RenderData.DirectionalLightSources.data(),
			m_RenderData.DirectionalLightSources.size() * sizeof(DirectionalLightSource));
		m_RenderData.DirectionalLightBuffer->UnMap();

		void* TextureData = m_RenderData.TextureBuffer->Map();
		memcpy(TextureData, m_RenderData.Textures.data(), m_RenderData.Textures.size() * sizeof(GLuint64));
		m_RenderData.TextureBuffer->UnMap();
		
		void* MaterialData = m_RenderData.MaterialBuffer->Map();
		memcpy(MaterialData, m_RenderData.Materials.data(), m_RenderData.Materials.size() * sizeof(OpenGLMaterial));
		m_RenderData.MaterialBuffer->UnMap();

		for (auto& [handle, ID] : m_RenderData.TextureSlots)
			glMakeTextureHandleResidentARB(handle);

		DownSample();

		m_RenderData.DirectLightingPass->Use();

		m_RenderData.DirectLightingPass->SetUniformVec3("CameraPosition", m_RenderData.ReferenceCamera->GetPosition(), 1);
		m_RenderData.DirectLightingPass->SetUniformMat4("InverseProjection", m_RenderData.ReferenceCamera->GetInverseProjection());
		m_RenderData.DirectLightingPass->SetUniformMat4("InverseView", m_RenderData.ReferenceCamera->GetInverseView());
		m_RenderData.DirectLightingPass->SetUniformInt("nLightSources", (int)m_RenderData.DirectionalLightSources.size());

		m_RenderData.DownSampledColorbuffer->BindImage(0);
		m_RenderData.DownSampledReflectiveAndMetallic->BindImage(1);
		m_RenderData.DownSampledEmissionAndRoughness->BindImage(2);
		m_RenderData.DownSampledNormal->BindImage(3);
		m_RenderData.DownSampledDepthBuffer->Bind(4);
		m_RenderData.DirectionalLightBuffer->BindBase(6);

		m_RenderData.DirectLightingPass->Compute(m_RenderData.DownScaledWidth / 8,
			m_RenderData.DownScaledHeight / 8, 1);

		glNamedFramebufferReadBuffer(m_RenderData.DownSampledFramebuffer->Get(), GL_COLOR_ATTACHMENT0);
		glNamedFramebufferDrawBuffer(m_RenderData.FinalFramebuffer->Get(), GL_COLOR_ATTACHMENT0 );

		glBlitNamedFramebuffer(
			m_RenderData.DownSampledFramebuffer->Get(), 
			m_RenderData.FinalFramebuffer->Get(),
			0, 0, (GLint)m_RenderData.DownScaledWidth,  (GLint)m_RenderData.DownScaledHeight, 
			0, 0, (GLint)m_RenderData.FramebufferWidth, (GLint)m_RenderData.FramebufferHeight, 
			GL_COLOR_BUFFER_BIT, GL_LINEAR);


		m_RenderData.DirectionalLightSources.clear();
		m_RenderData.Materials.clear();

		for (auto& [handle, ID] : m_RenderData.TextureSlots)
			glMakeTextureHandleNonResidentARB(handle);

		m_RenderData.FrameIndex++;

		glViewport(0, 0,
			Application::GetMainWindow().GetWidth(),
			Application::GetMainWindow().GetHeight());


		
	}
	void Renderer::Shutdown()
	{
	}

	OpenGLMaterial Renderer::ConstructMaterial(const MeshInstanceBuffer& Buffer, const Ecs::MaterialComponent& Material)
	{
		OpenGLMaterial Mat{};

		Mat.Albedo = Material.Albedo;
		Mat.Emission = Material.EmissionColor * Material.EmissionPower;
		Mat.Metallic = Material.Metallic;
		Mat.Reflectivity = Material.Reflectivity;
		Mat.Roughness = Material.Roughness;

		GLuint64 GPUHandle{};

		if (Material.MaterialTexture)
		{
			GPUHandle = Material.MaterialTexture->GetGPUHandle();

			if (m_RenderData.TextureSlots.contains(GPUHandle))
				Mat.TextureIndex = (float)m_RenderData.TextureSlots[GPUHandle];
			else
			{
				m_RenderData.Textures.push_back(GPUHandle);
				m_RenderData.TextureSlots[GPUHandle] = m_RenderData.Textures.size() - 1;
				Mat.TextureIndex = (float)m_RenderData.Textures.size() - 1;
			}
		}
		else
		{
			GPUHandle = Buffer.GetDefaultTexture()->GetGPUHandle();

			if (m_RenderData.TextureSlots.contains(GPUHandle))
				Mat.TextureIndex = (float)m_RenderData.TextureSlots[GPUHandle];
			else
			{
				m_RenderData.Textures.push_back(GPUHandle);
				m_RenderData.TextureSlots[GPUHandle] = m_RenderData.Textures.size() - 1;
				Mat.TextureIndex = (float)m_RenderData.Textures.size() - 1;
			}
		}

		return Mat;
	}

	void Renderer::CreateTextures()
	{
		{
			TooGoodEngine::TextureData TextureData;
			TextureData.InternalFormat = TooGoodEngine::TextureFormat::RGBA32F;
			TextureData.Width = m_RenderData.FramebufferWidth;
			TextureData.Height = m_RenderData.FramebufferHeight;

			glm::vec4* TempData = new glm::vec4[m_RenderData.FramebufferWidth * m_RenderData.FramebufferHeight];
			for (uint32_t i = 0; i < m_RenderData.FramebufferWidth * m_RenderData.FramebufferHeight; i++)
				TempData[i] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

			m_RenderData.FinalImage = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);

			delete[] TempData;
		}

		{
			TooGoodEngine::TextureData TextureData;
			TextureData.InternalFormat = TooGoodEngine::TextureFormat::RGBA32F;
			TextureData.Width = m_RenderData.FramebufferWidth;
			TextureData.Height = m_RenderData.FramebufferHeight;

			glm::vec4* TempData = new glm::vec4[m_RenderData.FramebufferWidth * m_RenderData.FramebufferHeight];
			for (uint32_t i = 0; i < m_RenderData.FramebufferWidth * m_RenderData.FramebufferHeight; i++)
				TempData[i] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

			TextureData.Type = TextureType::Texture2DMultisample;
			TextureData.NumberOfSamples = 4;
			TextureData.TextureParamaters =
			{
			};

			m_RenderData.ColorBuffer = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);
			m_RenderData.ReflectiveAndMetallic = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);
			m_RenderData.EmissionAndRoughnessBuffer = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);
			m_RenderData.NormalBuffer = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);
			m_RenderData.PositionBuffer = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);

			TextureData.InternalFormat = TextureFormat::DEPTH_32F;
			TextureData.Type = TextureType::Texture2DMultisample;
			float* d = nullptr;

			m_RenderData.DepthBuffer = Texture::GenerateShared(d, TextureData);

			TextureData.InternalFormat = TooGoodEngine::TextureFormat::RGBA32F;

			m_RenderData.TestTexture = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);

			delete[] TempData;
		}

		{
			TooGoodEngine::TextureData TextureData;
			TextureData.InternalFormat = TooGoodEngine::TextureFormat::RGBA32F;
			TextureData.Width = m_RenderData.FramebufferWidth;
			TextureData.Height = m_RenderData.FramebufferHeight;

			glm::vec4* TempData = new glm::vec4[m_RenderData.FramebufferWidth * m_RenderData.FramebufferHeight];
			for (uint32_t i = 0; i < m_RenderData.FramebufferWidth * m_RenderData.FramebufferHeight; i++)
				TempData[i] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

			m_RenderData.ResizedColorbuffer = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);
			m_RenderData.ResizedEmissionAndRoughness = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);
			m_RenderData.ResizedNormalBuffer = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);
			m_RenderData.ResizedReflectiveAndMetallic = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);
			m_RenderData.ResizedPositionBuffer = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);


			TextureData.InternalFormat = TextureFormat::DEPTH_32F;
			m_RenderData.ResizedDepthBuffer = Texture::GenerateShared((float*)nullptr, TextureData);

			delete[] TempData;
		}

		{
			TooGoodEngine::TextureData TextureData;
			TextureData.InternalFormat = TooGoodEngine::TextureFormat::RGBA32F;
			TextureData.Width = m_RenderData.DownScaledWidth;
			TextureData.Height = m_RenderData.DownScaledHeight;

			glm::vec4* TempData = new glm::vec4[m_RenderData.DownScaledWidth * m_RenderData.DownScaledHeight];
			memset(TempData, 0, m_RenderData.DownScaledWidth * m_RenderData.DownScaledHeight);

			m_RenderData.AccumulationBuffer = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);
			m_RenderData.DownSampledColorbuffer = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);
			m_RenderData.DownSampledEmissionAndRoughness = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);
			m_RenderData.DownSampledNormal = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);
			m_RenderData.DownSampledReflectiveAndMetallic = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);
			m_RenderData.DownSampledPositionBuffer = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);


			TextureData.InternalFormat = TextureFormat::DEPTH_32F;
			float* d = nullptr;
			m_RenderData.DownSampledDepthBuffer = Texture::GenerateShared(d, TextureData);

			delete[] TempData;
		}
	}

	void Renderer::CreateFramebuffers()
	{
		float Width = (float)Application::GetMainWindow().GetWidth();
		float Height = (float)Application::GetMainWindow().GetHeight();

		{
			FramebufferData FBufferData;
			FBufferData.AttachmentList =
			{
				{Attachment(AttachmentType::Color, (int)Width, (int)Height), m_RenderData.ColorBuffer},
				{Attachment(AttachmentType::Color, (int)Width, (int)Height), m_RenderData.ReflectiveAndMetallic},
				{Attachment(AttachmentType::Color, (int)Width, (int)Height), m_RenderData.EmissionAndRoughnessBuffer},
				{Attachment(AttachmentType::Color, (int)Width, (int)Height), m_RenderData.NormalBuffer},
				{Attachment(AttachmentType::Depth, (int)Width, (int)Height), m_RenderData.DepthBuffer}

			};

			m_RenderData.RenderFramebuffer = Framebuffer::GenerateShared(FBufferData);

			uint32_t attachments[4] = { GL_COLOR_ATTACHMENT0,
										GL_COLOR_ATTACHMENT1,
										GL_COLOR_ATTACHMENT2,
										GL_COLOR_ATTACHMENT3};

			glNamedFramebufferDrawBuffers(m_RenderData.RenderFramebuffer->Get(), 4, attachments);

			FramebufferData ResizeFramebufferData;
			ResizeFramebufferData.AttachmentList =
			{
				{Attachment(AttachmentType::Color, (int)Width, (int)Height), m_RenderData.ResizedColorbuffer},
				{Attachment(AttachmentType::Color, (int)Width, (int)Height), m_RenderData.ResizedReflectiveAndMetallic},
				{Attachment(AttachmentType::Color, (int)Width, (int)Height), m_RenderData.ResizedEmissionAndRoughness},
				{Attachment(AttachmentType::Color, (int)Width, (int)Height), m_RenderData.ResizedNormalBuffer},
				{Attachment(AttachmentType::Depth, (int)Width, (int)Height), m_RenderData.ResizedDepthBuffer}
			};

			m_RenderData.ResizedFramebuffer = Framebuffer::GenerateShared(ResizeFramebufferData);

			glNamedFramebufferDrawBuffers(m_RenderData.ResizedFramebuffer->Get(), 4, attachments);


			FramebufferData DownsampledFramebufferData;
			DownsampledFramebufferData.AttachmentList =
			{
				{Attachment(AttachmentType::Color, (int)(m_RenderData.DownScaledWidth), (int)(m_RenderData.DownScaledHeight)), m_RenderData.DownSampledColorbuffer},
				{Attachment(AttachmentType::Color, (int)(m_RenderData.DownScaledWidth), (int)(m_RenderData.DownScaledHeight)), m_RenderData.DownSampledReflectiveAndMetallic},
				{Attachment(AttachmentType::Color, (int)(m_RenderData.DownScaledWidth), (int)(m_RenderData.DownScaledHeight)), m_RenderData.DownSampledEmissionAndRoughness},
				{Attachment(AttachmentType::Color, (int)(m_RenderData.DownScaledWidth), (int)(m_RenderData.DownScaledHeight)), m_RenderData.DownSampledNormal},

				{Attachment(AttachmentType::Depth, (int)(m_RenderData.DownScaledWidth), (int)m_RenderData.DownScaledHeight), m_RenderData.DownSampledDepthBuffer}
			};

			m_RenderData.DownSampledFramebuffer = Framebuffer::GenerateShared(DownsampledFramebufferData);

			glNamedFramebufferDrawBuffers(m_RenderData.DownSampledFramebuffer->Get(), 4, attachments);


			FramebufferData FinalFramebufferData;
			FinalFramebufferData.AttachmentList =
			{
				{Attachment(AttachmentType::Color, (int)Width, (int)Height), m_RenderData.FinalImage},
			};

			m_RenderData.FinalFramebuffer = Framebuffer::GenerateShared(FinalFramebufferData);
		}
	}

	void Renderer::DownSample()
	{
		//msaa -> plain

		glNamedFramebufferReadBuffer(m_RenderData.RenderFramebuffer->Get(), GL_COLOR_ATTACHMENT0);
		glNamedFramebufferDrawBuffer(m_RenderData.ResizedFramebuffer->Get(), GL_COLOR_ATTACHMENT0);

		glBlitNamedFramebuffer(
			m_RenderData.RenderFramebuffer->Get(),
			m_RenderData.ResizedFramebuffer->Get(),
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glNamedFramebufferReadBuffer(m_RenderData.RenderFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 1);
		glNamedFramebufferDrawBuffer(m_RenderData.ResizedFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 1);

		glBlitNamedFramebuffer(
			m_RenderData.RenderFramebuffer->Get(),
			m_RenderData.ResizedFramebuffer->Get(),
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glNamedFramebufferReadBuffer(m_RenderData.RenderFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 2);
		glNamedFramebufferDrawBuffer(m_RenderData.ResizedFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 2);

		glBlitNamedFramebuffer(
			m_RenderData.RenderFramebuffer->Get(),
			m_RenderData.ResizedFramebuffer->Get(),
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glNamedFramebufferReadBuffer(m_RenderData.RenderFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 3);
		glNamedFramebufferDrawBuffer(m_RenderData.ResizedFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 3);

		glBlitNamedFramebuffer(
			m_RenderData.RenderFramebuffer->Get(),
			m_RenderData.ResizedFramebuffer->Get(),
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glNamedFramebufferReadBuffer(m_RenderData.RenderFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 4);
		glNamedFramebufferDrawBuffer(m_RenderData.ResizedFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 4);

		glBlitNamedFramebuffer(
			m_RenderData.RenderFramebuffer->Get(),
			m_RenderData.ResizedFramebuffer->Get(),
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);



		glBlitNamedFramebuffer(
			m_RenderData.RenderFramebuffer->Get(),
			m_RenderData.ResizedFramebuffer->Get(),
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		//high res -> low res

		glNamedFramebufferReadBuffer(m_RenderData.ResizedFramebuffer->Get(), GL_COLOR_ATTACHMENT0);
		glNamedFramebufferDrawBuffer(m_RenderData.DownSampledFramebuffer->Get(), GL_COLOR_ATTACHMENT0);

		glBlitNamedFramebuffer(
			m_RenderData.ResizedFramebuffer->Get(),
			m_RenderData.DownSampledFramebuffer->Get(),
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			0, 0, m_RenderData.DownScaledWidth, m_RenderData.DownScaledHeight,
			GL_COLOR_BUFFER_BIT, GL_LINEAR);


		glNamedFramebufferReadBuffer(m_RenderData.ResizedFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 1);
		glNamedFramebufferDrawBuffer(m_RenderData.DownSampledFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 1);

		glBlitNamedFramebuffer(
			m_RenderData.ResizedFramebuffer->Get(),
			m_RenderData.DownSampledFramebuffer->Get(),
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			0, 0, m_RenderData.DownScaledWidth, m_RenderData.DownScaledHeight,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glNamedFramebufferReadBuffer(m_RenderData.ResizedFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 2);
		glNamedFramebufferDrawBuffer(m_RenderData.DownSampledFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 2);

		glBlitNamedFramebuffer(
			m_RenderData.ResizedFramebuffer->Get(),
			m_RenderData.DownSampledFramebuffer->Get(),
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			0, 0, m_RenderData.DownScaledWidth, m_RenderData.DownScaledHeight,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glNamedFramebufferReadBuffer(m_RenderData.ResizedFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 3);
		glNamedFramebufferDrawBuffer(m_RenderData.DownSampledFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 3);

		glBlitNamedFramebuffer(
			m_RenderData.ResizedFramebuffer->Get(),
			m_RenderData.DownSampledFramebuffer->Get(),
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			0, 0, m_RenderData.DownScaledWidth, m_RenderData.DownScaledHeight,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glNamedFramebufferReadBuffer(m_RenderData.ResizedFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 4);
		glNamedFramebufferDrawBuffer(m_RenderData.DownSampledFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 4);

		glBlitNamedFramebuffer(
			m_RenderData.ResizedFramebuffer->Get(),
			m_RenderData.DownSampledFramebuffer->Get(),
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			0, 0, m_RenderData.DownScaledWidth, m_RenderData.DownScaledHeight,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glBlitNamedFramebuffer(
			m_RenderData.ResizedFramebuffer->Get(),
			m_RenderData.DownSampledFramebuffer->Get(),
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			0, 0, m_RenderData.DownScaledWidth, m_RenderData.DownScaledHeight,
			GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	}

	InstanceID Renderer::AddUniqueMesh(const Mesh& data)
	{
		m_RenderData.GroupedMeshInstances.emplace_back(data);
		return m_RenderData.GroupedMeshInstances.size() - 1;
	}

	InstanceID Renderer::AddUniqueModel(const Model& data)
	{
		m_RenderData.GroupedModelInstances.emplace_back();
		m_RenderData.GroupedModelInstances.back().InstanceMeshs.reserve(data.MeshList.size());

		for (size_t i = 0; i < data.MeshList.size(); i++)
			m_RenderData.GroupedModelInstances.back().InstanceMeshs.emplace_back(data.MeshList[i]);
		

		return m_RenderData.GroupedModelInstances.size() - 1;
	}

	void Renderer::DrawMeshInstance(InstanceID id, const glm::mat4& Transform, const Ecs::MaterialComponent& Material)
	{
		TGE_FORCE_ASSERT(id < m_RenderData.GroupedMeshInstances.size(), "not a valid ID");

		m_RenderData.GroupedMeshInstances[id].PushData(Transform, Material);
	}

	void Renderer::DrawModelInstance(InstanceID id, const glm::mat4& Transforms, const Ecs::MaterialComponent& Material)
	{
		TGE_FORCE_ASSERT(id < m_RenderData.GroupedModelInstances.size(), "not a valid ID");

		for (size_t i = 0; i < m_RenderData.GroupedModelInstances[id].InstanceMeshs.size(); i++)
		{
			OpenGLMaterial Mat = ConstructMaterial(m_RenderData.GroupedModelInstances[id].InstanceMeshs[i], Material);
			m_RenderData.Materials.push_back(Mat);

			m_RenderData.GroupedModelInstances[id].InstanceMeshs[i].PushData(Transforms, Material);

			if (m_RenderData.Materials.size() >= m_RenderData.MaterialBuffer->GetSize() / sizeof(OpenGLMaterial))
				m_RenderData.MaterialBuffer->Resize((uint32_t)(m_RenderData.Materials.size() + 1) * 2);

			if (m_RenderData.Textures.size() >= m_RenderData.TextureBuffer->GetSize() / sizeof(GLuint64))
				m_RenderData.TextureBuffer->Resize((uint32_t)(m_RenderData.Textures.size() + 1) * 2);

		}
	}

	void Renderer::DrawModelInstance(InstanceID id, const glm::mat4& Transforms)
	{
		TGE_FORCE_ASSERT(id < m_RenderData.GroupedModelInstances.size(), "not a valid ID");

		for (size_t i = 0; i < m_RenderData.GroupedModelInstances[id].InstanceMeshs.size(); i++)
		{
			m_RenderData.GroupedModelInstances[id].InstanceMeshs[i].PushData(Transforms);
		}
	}

	void Renderer::ChangeScaledResolution(float NewScale)
	{
		m_RenderData.Scale = std::max(NewScale, 1.0f);

		m_RenderData.DownScaledWidth  = (uint32_t)((float)m_RenderData.FramebufferWidth / m_RenderData.Scale);
		m_RenderData.DownScaledHeight = (uint32_t)((float)m_RenderData.FramebufferHeight / m_RenderData.Scale);

		m_RenderData.AccumulationBuffer = nullptr;
		m_RenderData.DownSampledColorbuffer = nullptr;
		m_RenderData.DownSampledEmissionAndRoughness = nullptr;
		m_RenderData.DownSampledNormal = nullptr;
		m_RenderData.DownSampledReflectiveAndMetallic = nullptr;
		m_RenderData.DownSampledDepthBuffer = nullptr;
		m_RenderData.DownSampledFramebuffer = nullptr;


		{
			TooGoodEngine::TextureData TextureData2;
			TextureData2.InternalFormat = TooGoodEngine::TextureFormat::RGBA32F;
			TextureData2.Width = m_RenderData.DownScaledWidth;
			TextureData2.Height = m_RenderData.DownScaledHeight;

			glm::vec4* TempData = new glm::vec4[m_RenderData.DownScaledWidth * m_RenderData.DownScaledHeight];
			memset(TempData, 0, m_RenderData.DownScaledWidth * m_RenderData.DownScaledHeight);
			
			m_RenderData.AccumulationBuffer = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData2);
			m_RenderData.DownSampledColorbuffer = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData2);
			m_RenderData.DownSampledEmissionAndRoughness = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData2);
			m_RenderData.DownSampledNormal = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData2);
			m_RenderData.DownSampledReflectiveAndMetallic = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData2);

			TextureData2.InternalFormat = TextureFormat::DEPTH_32F;
			float* d = nullptr;
			m_RenderData.DownSampledDepthBuffer = Texture::GenerateShared(d, TextureData2);

			delete[] TempData;
		}

		{
			FramebufferData DownsampledFramebufferData;
			DownsampledFramebufferData.AttachmentList =
			{
				{Attachment(AttachmentType::Color, m_RenderData.DownScaledWidth, m_RenderData.DownScaledHeight), m_RenderData.DownSampledColorbuffer},
				{Attachment(AttachmentType::Color, m_RenderData.DownScaledWidth, m_RenderData.DownScaledHeight), m_RenderData.DownSampledReflectiveAndMetallic},
				{Attachment(AttachmentType::Color, m_RenderData.DownScaledWidth, m_RenderData.DownScaledHeight), m_RenderData.DownSampledEmissionAndRoughness},
				{Attachment(AttachmentType::Color, m_RenderData.DownScaledWidth, m_RenderData.DownScaledHeight), m_RenderData.DownSampledNormal},


				{Attachment(AttachmentType::Depth, m_RenderData.DownScaledWidth, m_RenderData.DownScaledHeight), m_RenderData.DownSampledDepthBuffer}
			};

			m_RenderData.DownSampledFramebuffer = Framebuffer::GenerateShared(DownsampledFramebufferData);

			uint32_t attachments[4] = { GL_COLOR_ATTACHMENT0,
										GL_COLOR_ATTACHMENT1,
										GL_COLOR_ATTACHMENT2,
										GL_COLOR_ATTACHMENT3 };

			glNamedFramebufferDrawBuffers(m_RenderData.DownSampledFramebuffer->Get(), 4, attachments);
		}

	}

	void Renderer::ChangeMultiSampleRate(int NewRate)
	{
		//TODO
	}

	void Renderer::AddDirectionalLight(const DirectionalLightSource& Src)
	{
		if (m_RenderData.CurrentLightBufferCapacity <= m_RenderData.DirectionalLightSources.size())
		{
			m_RenderData.CurrentLightBufferCapacity *= 2;
			m_RenderData.DirectionalLightBuffer->Resize((uint32_t)m_RenderData.CurrentLightBufferCapacity * sizeof(DirectionalLightSource));
		}

		m_RenderData.DirectionalLightSources.push_back(Src);
	}

	//TESTING
	void Renderer::DrawPrimitiveQuad(const glm::vec3& Pos)
	{
		Ecs::MaterialComponent material(glm::vec4(1.0f), glm::vec3(0.0f), glm::vec3(0.0f), 1.0f, 0.0f, m_RenderData.TestTexture);
		material.Metallic = 1.0f;

		glm::mat4 Transform = glm::identity<glm::mat4>();
		Transform = glm::translate(Transform, Pos)
			* glm::rotate(Transform, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(Transform, glm::vec3(1.0f, 1.0f, 1.0f));

		m_RenderData.GroupedMeshInstances[RenderData::QuadInstanceID].PushData(Transform, material);
	}

	void Renderer::DrawPrimitiveQuad(const glm::mat4& Transform, const Ecs::MaterialComponent& MaterialComponent)
	{
		m_RenderData.GroupedMeshInstances[RenderData::QuadInstanceID].PushData(Transform, MaterialComponent);
	}
	
}