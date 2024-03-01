#include "pch.h"
#include "Renderer.h"

#include "Utils/Clock.h"

#include <utility>

namespace TooGoodEngine {

	//ty stack overflow
	static void debugVAOState(std::string baseMessage)
	{
		baseMessage.append(" ... querying VAO state:\n");
		int vab, eabb, eabbs, mva, isOn(1), vaabb;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vab);
		glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &eabb);
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &eabbs);

		baseMessage.append("  VAO: " + std::to_string(vab) + "\n");
		baseMessage.append("  IBO: " + std::to_string(eabb) + ", size=" + std::to_string(eabbs) + "\n");

		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &mva);
		for (unsigned i = 0; i < mva; ++i)
		{
			glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &isOn);
			if (isOn)
			{
				glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vaabb);
				baseMessage.append("  attrib #" + std::to_string(i) + ": VBO=" + std::to_string(vaabb) + "\n");
			}
		}
		
		TGE_LOG_INFO(baseMessage.c_str());
	}


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
		m_Count(data.Indicies.size()), m_DefaultTexture(data.Material.Texture), m_MeshCopy(data)
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
				Mat.TextureIndex = m_TextureSlots[GPUHandle];
			else
			{
				if (m_TextureBufferSize <= m_TextureSlots.size() * sizeof(uint64_t))
				{
					m_InstanceTextureBuffer.UnMap();

					m_TextureBufferSize = sizeof(GLuint64) * ((m_TextureSlots.size() + 1) * 2);
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
				Mat.TextureIndex = m_TextureSlots[GPUHandle];
			else
			{
				if (m_TextureBufferSize <= m_TextureSlots.size() * sizeof(uint64_t))
				{
					m_InstanceTextureBuffer.UnMap();

					m_TextureBufferSize = sizeof(GLuint64) * ((m_TextureSlots.size() + 1) * 2);
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

			m_MeshInstanceBufferSize = SizeOfMap * 2;
			
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

			m_MeshInstanceBufferSize = SizeOfMap * 2;

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
		m_RenderData.BoundingVolumeHierarchy = std::make_unique<BVHBuilder>();

		float Width = Application::GetMainWindow().GetWidth();
		float Height = Application::GetMainWindow().GetHeight();

		m_RenderData.FramebufferWidth = Width;
		m_RenderData.FramebufferHeight = Height;

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		glEnable(GL_DEPTH_TEST);

		{
			TooGoodEngine::TextureData TextureData;
			TextureData.InternalFormat = TooGoodEngine::TextureFormat::RGBA32F;
			TextureData.Width = Width;
			TextureData.Height = Height;

			glm::vec4* TempData = new glm::vec4[Width * Height];
			for (int i = 0; i < Width * Height; i++)
				TempData[i] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

			m_RenderData.FinalImage = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);

			delete[] TempData;
		}

		{
			TooGoodEngine::TextureData TextureData;
			TextureData.InternalFormat = TooGoodEngine::TextureFormat::RGBA32F;
			TextureData.Width = Width;
			TextureData.Height = Height;

			glm::vec4* TempData = new glm::vec4[Width * Height];
			for (int i = 0; i < Width * Height; i++)
				TempData[i] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

			TextureData.Type = TextureType::Texture2DMultisample;
			TextureData.NumberOfSamples = 4;
			TextureData.TextureParamaters = 
			{
			};
			m_RenderData.ColorBuffer =					Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);
			
			TextureData.Type = TextureType::Texture2DMultisample;
			TextureData.NumberOfSamples = 4;
			TextureData.TextureParamaters =
			{
			};
			m_RenderData.ReflectiveAndMetallic =		Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);
			m_RenderData.EmissionAndRoughnessBuffer =	Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);
			m_RenderData.NormalBuffer = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);

			TextureData.InternalFormat = TextureFormat::DEPTH_32F;
			float* d = nullptr;
			TextureData.Type = TextureType::Texture2DMultisample;
			TextureData.NumberOfSamples = 4;
			TextureData.TextureParamaters =
			{
			};
			m_RenderData.DepthBuffer = Texture::GenerateShared(d, TextureData);

			TextureData.InternalFormat = TooGoodEngine::TextureFormat::RGBA32F;

			m_RenderData.TestTexture = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);

			delete[] TempData;

			m_RenderData.FramebufferWidth = Width;
			m_RenderData.FramebufferHeight = Height;
		}

		{
			TooGoodEngine::TextureData TextureData;
			TextureData.InternalFormat = TooGoodEngine::TextureFormat::RGBA32F;
			TextureData.Width = Width;
			TextureData.Height = Height;

			glm::vec4* TempData = new glm::vec4[Width * Height];
			for (int i = 0; i < Width * Height; i++)
				TempData[i] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

			m_RenderData.ResizedColorbuffer = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);
			m_RenderData.ResizedEmissionAndRoughness = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);
			m_RenderData.ResizedNormalBuffer = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);
			m_RenderData.ResizedReflectiveAndMetallic = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);


			TextureData.InternalFormat = TextureFormat::DEPTH_32F;
			m_RenderData.ResizedDepthBuffer = Texture::GenerateShared((float*)nullptr, TextureData);

			delete[] TempData;
		}

		{
			TooGoodEngine::TextureData TextureData2;
			TextureData2.InternalFormat = TooGoodEngine::TextureFormat::RGBA32F;
			TextureData2.Width = Width / m_RenderData.Scale;
			TextureData2.Height = Height / m_RenderData.Scale;

			glm::vec4* TempData = new glm::vec4[(m_RenderData.FramebufferWidth / m_RenderData.Scale) * (m_RenderData.FramebufferHeight / m_RenderData.Scale)];
			memset(TempData, 0, (m_RenderData.FramebufferWidth / m_RenderData.Scale) * (m_RenderData.FramebufferHeight / m_RenderData.Scale));

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
			TooGoodEngine::TextureData TextureData2;
			TextureData2.InternalFormat = TooGoodEngine::TextureFormat::RGBA8;
			TextureData2.Width = Width / m_RenderData.Scale;
			TextureData2.Height = Height / m_RenderData.Scale;

			uint32_t* TempData = new uint32_t[(m_RenderData.FramebufferWidth / m_RenderData.Scale) * (m_RenderData.FramebufferHeight / m_RenderData.Scale)];
			memset(TempData, 0, (m_RenderData.FramebufferWidth / m_RenderData.Scale)* (m_RenderData.FramebufferHeight / m_RenderData.Scale));

			m_RenderData.ShadowMap = Texture::GenerateShared(TempData, TextureData2);

			delete[] TempData;
		}

		{
			FramebufferData FBufferData;
			FBufferData.AttachmentList =
			{
				{Attachment(AttachmentType::Color, Width, Height), m_RenderData.ColorBuffer},
				{Attachment(AttachmentType::Color, Width, Height), m_RenderData.ReflectiveAndMetallic},
				{Attachment(AttachmentType::Color, Width, Height), m_RenderData.EmissionAndRoughnessBuffer},
				{Attachment(AttachmentType::Color, Width, Height), m_RenderData.NormalBuffer},

				{Attachment(AttachmentType::Depth, Width, Height), m_RenderData.DepthBuffer}
			};

			m_RenderData.RenderFramebuffer = Framebuffer::GenerateShared(FBufferData);

			uint32_t attachments[4] = { GL_COLOR_ATTACHMENT0,
									    GL_COLOR_ATTACHMENT1,
									    GL_COLOR_ATTACHMENT2,
										GL_COLOR_ATTACHMENT3 };

			glNamedFramebufferDrawBuffers(m_RenderData.RenderFramebuffer->Get(), 4, attachments);

			FramebufferData ResizeFramebufferData;
			ResizeFramebufferData.AttachmentList =
			{
				{Attachment(AttachmentType::Color, Width, Height), m_RenderData.ResizedColorbuffer},
				{Attachment(AttachmentType::Color, Width, Height), m_RenderData.ResizedReflectiveAndMetallic},
				{Attachment(AttachmentType::Color, Width, Height), m_RenderData.ResizedEmissionAndRoughness},
				{Attachment(AttachmentType::Color, Width, Height), m_RenderData.ResizedNormalBuffer},

				{Attachment(AttachmentType::Depth, Width, Height), m_RenderData.ResizedDepthBuffer}
			};

			m_RenderData.ResizedFramebuffer = Framebuffer::GenerateShared(ResizeFramebufferData);

			glNamedFramebufferDrawBuffers(m_RenderData.ResizedFramebuffer->Get(), 4, attachments);


			FramebufferData DownsampledFramebufferData;
			DownsampledFramebufferData.AttachmentList =
			{
				{Attachment(AttachmentType::Color, Width / m_RenderData.Scale, Height / m_RenderData.Scale), m_RenderData.DownSampledColorbuffer},
				{Attachment(AttachmentType::Color, Width / m_RenderData.Scale, Height / m_RenderData.Scale), m_RenderData.DownSampledReflectiveAndMetallic},
				{Attachment(AttachmentType::Color, Width / m_RenderData.Scale, Height / m_RenderData.Scale), m_RenderData.DownSampledEmissionAndRoughness},
				{Attachment(AttachmentType::Color, Width / m_RenderData.Scale, Height / m_RenderData.Scale), m_RenderData.DownSampledNormal},


				{Attachment(AttachmentType::Depth, Width / m_RenderData.Scale, Height / m_RenderData.Scale), m_RenderData.DownSampledDepthBuffer}
			};

			m_RenderData.DownSampledFramebuffer = Framebuffer::GenerateShared(DownsampledFramebufferData);

			glNamedFramebufferDrawBuffers(m_RenderData.DownSampledFramebuffer->Get(), 4, attachments);


			FramebufferData FinalFramebufferData;
			FinalFramebufferData.AttachmentList =
			{
				{Attachment(AttachmentType::Color, Width, Height), m_RenderData.FinalImage},
			};

			m_RenderData.FinalFramebuffer = Framebuffer::GenerateShared(FinalFramebufferData);
		}

		{
			std::map<GLenum, std::string_view> ShaderList;
			ShaderList[GL_VERTEX_SHADER] =	   "../Engine-Core/src/Graphics/Shaders/RendererShaders/ColorPassVertex.glsl";
			ShaderList[GL_FRAGMENT_SHADER] =   "../Engine-Core/src/Graphics/Shaders/RendererShaders/ColorPassFragment.glsl";

			m_RenderData.GBufferPass = Shader::Generate(ShaderList);
		}

		{
			std::map<GLenum, std::string_view> ShaderList;
			ShaderList[GL_COMPUTE_SHADER] = "../Engine-Core/src/Graphics/Shaders/RendererShaders/ShadowPass.glsl";

			m_RenderData.ShadowPass = Shader::Generate(ShaderList);
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

			InstanceData.Vertices	  = QuadVerticies;
			InstanceData.Indicies	  = QuadIndicies;
			InstanceData.Material	  = {};
			
			InstanceData.Material.Albedo		= glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			InstanceData.Material.EmissionColor = glm::vec3(1.0f, 1.0f, 1.0f);
			InstanceData.Material.Reflectivity	= glm::vec3(1.0f, 1.0f, 1.0f);
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
	}
	void Renderer::Begin(BaseCamera& RefCamera)
	{
		m_RenderData.ReferenceCamera = &RefCamera;
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

		m_RenderData.BoundingVolumeHierarchy->Build(m_RenderData.BoundingBuildType);
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
			0, 0, m_RenderData.FramebufferWidth / m_RenderData.Scale, m_RenderData.FramebufferHeight / m_RenderData.Scale,
			GL_COLOR_BUFFER_BIT, GL_LINEAR);


		glNamedFramebufferReadBuffer(m_RenderData.ResizedFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 1);
		glNamedFramebufferDrawBuffer(m_RenderData.DownSampledFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 1);

		glBlitNamedFramebuffer(
			m_RenderData.ResizedFramebuffer->Get(),
			m_RenderData.DownSampledFramebuffer->Get(),
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			0, 0, m_RenderData.FramebufferWidth / m_RenderData.Scale, m_RenderData.FramebufferHeight / m_RenderData.Scale,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glNamedFramebufferReadBuffer(m_RenderData.ResizedFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 2);
		glNamedFramebufferDrawBuffer(m_RenderData.DownSampledFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 2);

		glBlitNamedFramebuffer(
			m_RenderData.ResizedFramebuffer->Get(),
			m_RenderData.DownSampledFramebuffer->Get(),
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			0, 0, m_RenderData.FramebufferWidth / m_RenderData.Scale, m_RenderData.FramebufferHeight / m_RenderData.Scale,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glNamedFramebufferReadBuffer(m_RenderData.ResizedFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 3);
		glNamedFramebufferDrawBuffer(m_RenderData.DownSampledFramebuffer->Get(), GL_COLOR_ATTACHMENT0 + 3);

		glBlitNamedFramebuffer(
			m_RenderData.ResizedFramebuffer->Get(),
			m_RenderData.DownSampledFramebuffer->Get(),
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			0, 0, m_RenderData.FramebufferWidth / m_RenderData.Scale, m_RenderData.FramebufferHeight / m_RenderData.Scale,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glBlitNamedFramebuffer(
			m_RenderData.ResizedFramebuffer->Get(),
			m_RenderData.DownSampledFramebuffer->Get(),
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight,
			0, 0, m_RenderData.FramebufferWidth / m_RenderData.Scale, m_RenderData.FramebufferHeight / m_RenderData.Scale,
			GL_DEPTH_BUFFER_BIT, GL_NEAREST);


		m_RenderData.ShadowPass->Use();

		m_RenderData.ShadowPass->SetUniformMat4("InverseProjection", m_RenderData.ReferenceCamera->GetInverseProjection());
		m_RenderData.ShadowPass->SetUniformMat4("InverseView", m_RenderData.ReferenceCamera->GetInverseView());

		m_RenderData.DownSampledColorbuffer->BindImage(0);
		m_RenderData.DownSampledDepthBuffer->Bind(1);
		m_RenderData.ShadowMap->BindImage(2);
		m_RenderData.DownSampledNormal->BindImage(3);

		m_RenderData.BoundingVolumeHierarchy->Dispatch(3, 4);

		m_RenderData.ShadowPass->Compute((m_RenderData.FramebufferWidth/m_RenderData.Scale) / 8, 
			(m_RenderData.FramebufferHeight/m_RenderData.Scale) / 8, 1);

		m_RenderData.DirectLightingPass->Use();

		m_RenderData.DirectLightingPass->SetUniformVec3("CameraPosition", m_RenderData.ReferenceCamera->GetPosition(), 1);
		m_RenderData.DirectLightingPass->SetUniformMat4("InverseProjection", m_RenderData.ReferenceCamera->GetInverseProjection());
		m_RenderData.DirectLightingPass->SetUniformMat4("InverseView", m_RenderData.ReferenceCamera->GetInverseView());

		m_RenderData.DownSampledColorbuffer->BindImage(0);
		m_RenderData.DownSampledReflectiveAndMetallic->BindImage(1);
		m_RenderData.DownSampledEmissionAndRoughness->BindImage(2);
		m_RenderData.DownSampledNormal->BindImage(3);
		m_RenderData.DownSampledDepthBuffer->Bind(4);
		m_RenderData.ShadowMap->BindImage(5);

		m_RenderData.DirectLightingPass->Compute((m_RenderData.FramebufferWidth / m_RenderData.Scale) / 8,
			(m_RenderData.FramebufferHeight / m_RenderData.Scale) / 8, 1);

		glNamedFramebufferReadBuffer(m_RenderData.DownSampledFramebuffer->Get(), GL_COLOR_ATTACHMENT0);
		glNamedFramebufferDrawBuffer(m_RenderData.FinalFramebuffer->Get(), GL_COLOR_ATTACHMENT0 );

		glBlitNamedFramebuffer(
			m_RenderData.DownSampledFramebuffer->Get(), 
			m_RenderData.FinalFramebuffer->Get(),
			0, 0, m_RenderData.FramebufferWidth/m_RenderData.Scale, m_RenderData.FramebufferHeight/m_RenderData.Scale, 
			0, 0, m_RenderData.FramebufferWidth, m_RenderData.FramebufferHeight, 
			GL_COLOR_BUFFER_BIT, GL_LINEAR);


		glViewport(0, 0,
			Application::GetMainWindow().GetWidth(),
			Application::GetMainWindow().GetHeight());
		
	}
	void Renderer::Shutdown()
	{
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
			
			m_RenderData.GroupedModelInstances[id].InstanceMeshs[i].PushData(Transforms, Material);
			m_RenderData.BoundingVolumeHierarchy->AddMesh(m_RenderData.GroupedModelInstances[id].InstanceMeshs[i].GetCopy(),
				Transforms);
		}
	}

	void Renderer::DrawModelInstance(InstanceID id, const glm::mat4& Transforms)
	{
		TGE_FORCE_ASSERT(id < m_RenderData.GroupedModelInstances.size(), "not a valid ID");

		for (size_t i = 0; i < m_RenderData.GroupedModelInstances[id].InstanceMeshs.size(); i++)
		{
			m_RenderData.GroupedModelInstances[id].InstanceMeshs[i].PushData(Transforms);
			m_RenderData.BoundingVolumeHierarchy->AddMesh(m_RenderData.GroupedModelInstances[id].InstanceMeshs[i].GetCopy(),
				Transforms);
		}
	}

	void Renderer::ChangeScaledResolution(float NewScale)
	{
		m_RenderData.Scale = std::max(NewScale, 1.0f);

		m_RenderData.DownSampledColorbuffer = nullptr;
		m_RenderData.DownSampledEmissionAndRoughness = nullptr;
		m_RenderData.DownSampledNormal = nullptr;
		m_RenderData.DownSampledReflectiveAndMetallic = nullptr;
		m_RenderData.DownSampledDepthBuffer = nullptr;
		m_RenderData.DownSampledFramebuffer = nullptr;

		m_RenderData.ShadowMap = nullptr;

		{
			TooGoodEngine::TextureData TextureData2;
			TextureData2.InternalFormat = TooGoodEngine::TextureFormat::RGBA32F;
			TextureData2.Width = m_RenderData.FramebufferWidth / m_RenderData.Scale;
			TextureData2.Height = m_RenderData.FramebufferHeight / m_RenderData.Scale;

			glm::vec4* TempData = new glm::vec4[(m_RenderData.FramebufferWidth / m_RenderData.Scale) * (m_RenderData.FramebufferHeight / m_RenderData.Scale)];
			memset(TempData, 0, (m_RenderData.FramebufferWidth / m_RenderData.Scale) * (m_RenderData.FramebufferHeight / m_RenderData.Scale));

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
			TooGoodEngine::TextureData TextureData2;
			TextureData2.InternalFormat = TooGoodEngine::TextureFormat::RGBA8;
			TextureData2.Width = m_RenderData.FramebufferWidth / m_RenderData.Scale;
			TextureData2.Height = m_RenderData.FramebufferHeight / m_RenderData.Scale;

			uint32_t* TempData = new uint32_t[(m_RenderData.FramebufferWidth / m_RenderData.Scale) * (m_RenderData.FramebufferHeight / m_RenderData.Scale)];
			memset(TempData, 0, (m_RenderData.FramebufferWidth / m_RenderData.Scale) * (m_RenderData.FramebufferHeight / m_RenderData.Scale));

			m_RenderData.ShadowMap = Texture::GenerateShared(TempData, TextureData2);

			delete[] TempData;
		}

		{
			FramebufferData DownsampledFramebufferData;
			DownsampledFramebufferData.AttachmentList =
			{
				{Attachment(AttachmentType::Color, m_RenderData.FramebufferWidth / m_RenderData.Scale, m_RenderData.FramebufferHeight / m_RenderData.Scale), m_RenderData.DownSampledColorbuffer},
				{Attachment(AttachmentType::Color, m_RenderData.FramebufferWidth / m_RenderData.Scale, m_RenderData.FramebufferHeight / m_RenderData.Scale), m_RenderData.DownSampledReflectiveAndMetallic},
				{Attachment(AttachmentType::Color, m_RenderData.FramebufferWidth / m_RenderData.Scale, m_RenderData.FramebufferHeight / m_RenderData.Scale), m_RenderData.DownSampledEmissionAndRoughness},
				{Attachment(AttachmentType::Color, m_RenderData.FramebufferWidth / m_RenderData.Scale, m_RenderData.FramebufferHeight / m_RenderData.Scale), m_RenderData.DownSampledNormal},


				{Attachment(AttachmentType::Depth, m_RenderData.FramebufferWidth / m_RenderData.Scale, m_RenderData.FramebufferHeight / m_RenderData.Scale), m_RenderData.DownSampledDepthBuffer}
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

	void Renderer::ChangeBVHBuildType(BuildType NewBuildType)
	{
		m_RenderData.BoundingBuildType = NewBuildType;
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