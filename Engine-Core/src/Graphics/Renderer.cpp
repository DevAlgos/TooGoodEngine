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

	/*positions							normals		Texture Coordinates
	* data = { 0.0f, 0.0f, 0.0f,	1.0f, 0.0f, 0.0f,    1.0f, 1.0f }*/

	InstanceBuffer::InstanceBuffer(const InstanceBufferData& data)
		: m_InstanceVAO(),
		m_InstanceVertexBuffer(BufferType::VertexBuffer, { data.Data.data(),  data.Data.size() * sizeof(GLfloat), GL_STATIC_DRAW}),
		m_InstanceBuffer(BufferType::VertexBuffer, { nullptr, m_InstanceBufferSize, GL_DYNAMIC_DRAW }),
		m_InstanceSecondBuffer(BufferType::VertexBuffer, { nullptr, m_InstanceBufferSize, GL_DYNAMIC_DRAW}),
		m_InstanceThirdBuffer(BufferType::VertexBuffer, { nullptr, m_InstanceBufferSize, GL_DYNAMIC_DRAW }),
		m_InstanceIndexBuffer(BufferType::IndexBuffer, { data.Indicies.data(), data.Indicies.size() * sizeof(GLuint), GL_STATIC_DRAW}),
		m_InstanceUniformTextureBuffer(BufferType::ShaderStorageBuffer, {nullptr, m_TextureBufferSize, GL_DYNAMIC_DRAW }),
		m_Count(data.Indicies.size())
	{
		std::vector<AttributeType> InstanceVertexAttribs =
		{
			AttributeType::FLOAT_3, //vertex
			AttributeType::FLOAT_3, //normal
			AttributeType::FLOAT_2  //texture coordinate
		};

		std::vector<AttributeType> InstanceBufferAttribs =
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
			{&m_InstanceBuffer, InstanceBufferAttribs, true} //true meaning is instanced
		};

		m_AttributeDataCopy = m_InstanceVAO.AttachAttribToBuffer(AttribData);

		m_InstanceVAO.AttachIndexBuffer(m_InstanceIndexBuffer);

		m_BufferMapBase = (float*)m_InstanceBuffer.MapBufferRange();
		m_TextureBufferBase = (GLuint64*)m_InstanceUniformTextureBuffer.MapBufferRange();
	}

	InstanceBuffer::~InstanceBuffer()
	{
		if (m_CurrentBufferIndex == 0)
			m_InstanceBuffer.UnMap();
		else if (m_CurrentBufferIndex == 1)
			m_InstanceSecondBuffer.UnMap();
		else
			m_InstanceThirdBuffer.UnMap();
	}
	

	void InstanceBuffer::PushData(const glm::mat4& Transform, const Ecs::MaterialComponent& Material)
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
				if (m_TextureBufferSize <= m_TextureSlots.size())
				{
					m_InstanceUniformTextureBuffer.UnMap();

					m_TextureBufferSize = sizeof(GLuint64) * ((m_TextureSlots.size() + 1) * 2);
					m_InstanceUniformTextureBuffer.Resize(m_TextureBufferSize);

					m_TextureBufferBase = (GLuint64*)m_InstanceUniformTextureBuffer.MapBufferRange();
				}

				m_TextureBufferBase[m_CurrentTextureSlot++] = GPUHandle;
				m_TextureSlots[GPUHandle] = m_CurrentTextureSlot;
				Mat.TextureIndex = (float)m_CurrentTextureSlot;
			}
		}

		Mat.TextureIndex = 0;


		const float* MatPtr = glm::value_ptr(Mat.Albedo);
		int size = sizeof(OpenGLMaterial) / sizeof(Mat.Albedo.x);
		memcpy(m_BufferMapBase + m_MapOffset, MatPtr, (size_t)size * sizeof(float));
		m_MapOffset += size;

		const float* Source = (const float*)glm::value_ptr(Transform);
		memcpy(m_BufferMapBase + m_MapOffset, Source, 16 * sizeof(float));
		m_MapOffset += 16;

		const size_t SizeOfMap = m_MapOffset * sizeof(float);

		if (SizeOfMap >= m_InstanceBufferSize)
		{
			if (m_CurrentBufferIndex == 0)
				m_InstanceBuffer.UnMap();
			else if (m_CurrentBufferIndex == 1)
				m_InstanceSecondBuffer.UnMap();
			else
				m_InstanceThirdBuffer.UnMap();

			m_InstanceBufferSize = SizeOfMap * 2;
			
			m_InstanceBuffer.Resize(m_InstanceBufferSize);
			m_InstanceSecondBuffer.Resize(m_InstanceBufferSize);
			m_InstanceThirdBuffer.Resize(m_InstanceBufferSize);

			if (m_CurrentBufferIndex == 0)
			{
				m_BufferMapBase = (float*)m_InstanceBuffer.MapBufferRange();
				m_InstanceVAO.AttachVertexBuffer(m_InstanceBuffer, 1, m_AttributeDataCopy.Offsets[1], m_AttributeDataCopy.Strides[1]);

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

	void InstanceBuffer::BeginInstanceBatch()
	{
		m_InstanceUniformTextureBuffer.UnMap();

		if (m_CurrentBufferIndex == 0)
			m_InstanceBuffer.UnMap();
		else if (m_CurrentBufferIndex == 1)
			m_InstanceSecondBuffer.UnMap();
		else
			m_InstanceThirdBuffer.UnMap();

		for (auto& [handle, ID] : m_TextureSlots)
			glMakeTextureHandleResidentARB(handle);

		m_InstanceUniformTextureBuffer.BindBase(0);

		m_InstanceVAO.Bind();

	}

	void InstanceBuffer::EndInstanceBatch()
	{
	 	for (auto& [handle, ID] : m_TextureSlots)
			glMakeTextureHandleNonResidentARB(handle);

		m_InstanceCount = 0;
		m_CurrentTextureSlot = 0;
		m_MapOffset = 0;

		m_TextureSlots.clear();

		m_TextureBufferBase = (GLuint64*)m_InstanceUniformTextureBuffer.MapBufferRange();

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
			m_BufferMapBase = (float*)m_InstanceBuffer.MapBufferRange();
			m_InstanceVAO.AttachVertexBuffer(m_InstanceBuffer, 1, m_AttributeDataCopy.Offsets[1], m_AttributeDataCopy.Strides[1]);
			m_CurrentBufferIndex = 0;
		}
	}


	RenderData Renderer::m_RenderData;

	void Renderer::Init()
	{
		float Width = Application::GetMainWindow().GetWidth();
		float Height = Application::GetMainWindow().GetHeight();

		{
			TooGoodEngine::TextureData TextureData;
			TextureData.InternalFormat = TooGoodEngine::TextureFormat::RGBA32F;
			TextureData.Width = Width;
			TextureData.Height = Height;

			glm::vec4* TempData = new glm::vec4[Width * Height];
			for (int i = 0; i < Width * Height; i++)
				TempData[i] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

			m_RenderData.ColorBuffer = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);

			TextureData.InternalFormat = TextureFormat::DEPTH_32F;
			float* d = nullptr;
			m_RenderData.DepthBuffer = Texture::GenerateShared(d, TextureData);

			TextureData.InternalFormat = TooGoodEngine::TextureFormat::RGBA32F;

			m_RenderData.TestTexture = Texture::GenerateShared(glm::value_ptr(*TempData), TextureData);

			delete[] TempData;
		}

		{
			FramebufferData BufferData;
			BufferData.AttachmentList =
			{
				{Attachment(AttachmentType::Color, Width, Height), m_RenderData.ColorBuffer},
				{Attachment(AttachmentType::Depth, Width, Height), m_RenderData.DepthBuffer}
			};

			m_RenderData.RenderFramebuffer = Framebuffer::GenerateShared(BufferData);
		}

		{
			std::map<GLenum, std::string_view> ShaderList;
			ShaderList[GL_VERTEX_SHADER] =	   "../Engine-Core/src/Graphics/Shaders/RendererShaders/ColorPassVertex.glsl";
			ShaderList[GL_FRAGMENT_SHADER] =   "../Engine-Core/src/Graphics/Shaders/RendererShaders/ColorPassFragment.glsl";

			m_RenderData.InitialColorPass = Shader::Generate(ShaderList);
		}

		/*
		* Instance ID 0 will correspond to a primitive quad instance
		*/

		{
			InstanceBufferData InstanceData;

			InstanceData.Data	  = QuadVerticies;
			InstanceData.Indicies = QuadIndicies;

			if (m_RenderData.GroupedInstances.size() <= 0)
				m_RenderData.GroupedInstances.reserve(30);

			m_RenderData.GroupedInstances.emplace_back(InstanceData);
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

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		
		
		for (auto& Instance : m_RenderData.GroupedInstances)
		{
			m_RenderData.InitialColorPass->Use();
			m_RenderData.InitialColorPass->setUniformMat4("ViewProjection", ViewProjectionMatrix);

			Instance.BeginInstanceBatch();

			glDrawElementsInstanced(
				GL_TRIANGLES,                     
				(GLuint)(Instance.GetCount()),     
				GL_UNSIGNED_INT,                   
				nullptr,                          
				(GLuint)(Instance.GetNumberOfInstances())
			);

			Instance.EndInstanceBatch();
		}

		m_RenderData.RenderFramebuffer->UnBind();
	}
	void Renderer::Shutdown()
	{
	}

	void Renderer::DrawPrimitiveQuad(const glm::vec3& Pos)
	{
		Ecs::MaterialComponent material(glm::vec4(1.0f), glm::vec3(0.0f), glm::vec3(0.0f), 1.0f, 0.0f, m_RenderData.TestTexture);
		material.Metallic = 1.0f;

		glm::mat4 Transform = glm::identity<glm::mat4>();
		Transform = glm::translate(Transform, Pos)
			* glm::rotate(Transform, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(Transform, glm::vec3(1.0f, 1.0f, 1.0f));

		m_RenderData.GroupedInstances[RenderData::QuadInstanceID].PushData(Transform, material);

	}

	void Renderer::DrawPrimitiveQuad(const glm::mat4& Transform, const Ecs::MaterialComponent& MaterialComponent)
	{
		m_RenderData.GroupedInstances[RenderData::QuadInstanceID].PushData(Transform, MaterialComponent);
	}

	
	
}