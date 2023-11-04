#include <pch.h>
#include "Buffers.h"

#include <ctype.h>

namespace
{
	static float QuadVertices[] = {
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};
}

namespace Graphics 
{

	BufferObject::BufferObject(const BufferType& Type, const BufferData& BufferData)
		: m_Data(BufferData), m_Type(Type)
	{
		Create();
	}
	BufferObject::~BufferObject()
	{
		glDeleteBuffers(1, &m_VBO);
	}
	void BufferObject::Create()
	{
		switch (m_Type)
		{
		case Graphics::BufferObject::BufferType::VertexBuffer:
			glCreateBuffers(1, &m_VBO);
			glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
			glBufferData(GL_ARRAY_BUFFER, m_Data.VertexSize, m_Data.data, m_Data.DrawType);
			break;
		case Graphics::BufferObject::BufferType::IndexBuffer:
			glCreateBuffers(1, &m_VBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Data.VertexSize, m_Data.data, m_Data.DrawType);
			break;
		case Graphics::BufferObject::BufferType::UniformBuffer:
			glCreateBuffers(1, &m_VBO);
			glBindBuffer(GL_UNIFORM_BUFFER, m_VBO);
			glBufferData(GL_UNIFORM_BUFFER, m_Data.VertexSize, m_Data.data, m_Data.DrawType);
			break;
		case Graphics::BufferObject::BufferType::ShaderStorageBuffer:
			glCreateBuffers(1, &m_VBO);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_VBO);
			glBufferData(GL_SHADER_STORAGE_BUFFER, m_Data.VertexSize, m_Data.data, m_Data.DrawType);
		default:
			break;
		}
	}
	void BufferObject::BindRange(const DynamicData& data)
	{
		if (data.data)
			LOGWARNING("Data initalized with values when not needed!");

		switch (m_Type)
		{
		case Graphics::BufferObject::BufferType::VertexBuffer:
			glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
			glBindBufferRange(GL_ARRAY_BUFFER, data.index, m_VBO, data.Offset, data.VertexSize);
			break;
		case Graphics::BufferObject::BufferType::IndexBuffer:
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBO);
			glBindBufferRange(GL_ELEMENT_ARRAY_BUFFER, data.index, m_VBO, data.Offset, data.VertexSize);
			break;
		case Graphics::BufferObject::BufferType::UniformBuffer:
			glBindBuffer(GL_UNIFORM_BUFFER, m_VBO);
			glBindBufferRange(GL_UNIFORM_BUFFER, data.index, m_VBO, data.Offset, data.VertexSize);
			break;
		case Graphics::BufferObject::BufferType::ShaderStorageBuffer:
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_VBO);
			glBindBufferRange(GL_SHADER_STORAGE_BUFFER, data.index, m_VBO, data.Offset, data.VertexSize);
			break;
		default:
			break;
		}
	}
	void BufferObject::PushData(const DynamicData& data)
	{
		if (!data.data)
			LOGERROR("Data is nullptr!");

		switch (m_Type)
		{
		case Graphics::BufferObject::BufferType::VertexBuffer:
			glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
			glBufferSubData(GL_ARRAY_BUFFER, data.Offset, data.VertexSize, data.data);
			break;
		case Graphics::BufferObject::BufferType::IndexBuffer:
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBO);
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, data.Offset, data.VertexSize, data.data);
			break;
		case Graphics::BufferObject::BufferType::UniformBuffer:
			glBindBuffer(GL_UNIFORM_BUFFER, m_VBO);
			glBufferSubData(GL_UNIFORM_BUFFER, data.Offset, data.VertexSize, data.data);
			break;
		case Graphics::BufferObject::BufferType::ShaderStorageBuffer:
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_VBO);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, data.Offset, data.VertexSize, data.data);
			break;
		default:
			break;
		}
	}
	void BufferObject::Bind()
	{
		switch (m_Type)
		{
		case Graphics::BufferObject::BufferType::VertexBuffer:
			glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
			break;
		case Graphics::BufferObject::BufferType::IndexBuffer:
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBO);
			break;
		case Graphics::BufferObject::BufferType::UniformBuffer:
			glBindBuffer(GL_UNIFORM_BUFFER, m_VBO);
			break;
		default:
			break;
		}
	}



	Framebuffer::Framebuffer(std::vector<Attachment> Attachments, std::map<GLenum, const char*> ShaderList)
		: m_Framebuffer(0)
	{
		FramebufferShader = std::make_unique<Shader>(ShaderList);
		
		FramebufferVAO = std::make_unique<VertexArrayObject>();
		FramebufferVAO->Create();

		BufferData data { QuadVertices, sizeof(QuadVertices), GL_STATIC_DRAW};
		FramebufferVBO = std::make_unique<BufferObject>(BufferObject::BufferType::VertexBuffer, data);

		FramebufferVAO->AttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
		FramebufferVAO->AttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));

		if (Attachments.size() > 34) {
			LOGERROR("Bounds of framebuffer exceeded! Make a new framebuffer");
		}

		m_Renderbuffers.resize(Attachments.size());
		m_Textures.resize(Attachments.size());

		memset(m_Attachments.data(), 0, m_Attachments.size());

		glCreateFramebuffers(1, &m_Framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);


		for (size_t i = 0; i < Attachments.size(); i++)
			m_Attachments[i] = Attachments[i];

		NumberOfRenderTargets = Attachments.size();

		for (size_t i = 0; i < NumberOfRenderTargets; i++)
		{
			if (nColorAttachments > 32) {
				LOGERROR("To many color attachments, maximum supported is 32!");
				LOGERROR("Incomplete framebuffer");
				break;
			}
			else
			{
				switch (m_Attachments[i].Type)
				{
				case AttachmentType::Color:
					if (m_Attachments[i].isRenderable)
					{
						glCreateRenderbuffers(1, &m_Renderbuffers[RenderIndex]);
						glBindRenderbuffer(GL_RENDERBUFFER, m_Renderbuffers[RenderIndex]);
						glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, m_Attachments[i].Width, m_Attachments[i].Height);
						glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (GLenum)nColorAttachments, GL_RENDERBUFFER, m_Renderbuffers[RenderIndex]);
						RenderIndex++;
					}
					else
					{
						glCreateTextures(GL_TEXTURE_2D, 1, &m_Textures[TextureIndex]);
						glBindTexture(GL_TEXTURE_2D, m_Textures[TextureIndex]);
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Attachments[i].Width, m_Attachments[i].Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (GLenum)nColorAttachments, GL_TEXTURE_2D, m_Textures[TextureIndex], 0);
						TextureIndex++;
					}

					nColorAttachments++;
					break;
				case AttachmentType::Stencil:
					if (m_Attachments[i].isRenderable)
					{
						glCreateRenderbuffers(1, &m_Renderbuffers[RenderIndex]);
						glBindRenderbuffer(GL_RENDERBUFFER, m_Renderbuffers[RenderIndex]);
						glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, m_Attachments[i].Width, m_Attachments[i].Height);
						glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_Renderbuffers[RenderIndex]);
						RenderIndex++;
					}
					else
					{
						glCreateTextures(GL_TEXTURE_2D, 1, &m_Textures[TextureIndex]);
						glBindTexture(GL_TEXTURE_2D, m_Textures[TextureIndex]);
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Attachments[i].Width, m_Attachments[i].Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_Textures[TextureIndex], 0);
						TextureIndex++;
					}
					break;
				case AttachmentType::Depth:
					if (m_Attachments[i].isRenderable)
					{
						glCreateRenderbuffers(1, &m_Renderbuffers[RenderIndex]);
						glBindRenderbuffer(GL_RENDERBUFFER, m_Renderbuffers[RenderIndex]);
						glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, m_Attachments[i].Width, m_Attachments[i].Height);
						glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_Renderbuffers[RenderIndex]);
						RenderIndex++;
					}
					else
					{
						glCreateTextures(GL_TEXTURE_2D, 1, &m_Textures[TextureIndex]);
						glBindTexture(GL_TEXTURE_2D, m_Textures[TextureIndex]);
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Attachments[i].Width, m_Attachments[i].Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_Textures[TextureIndex], 0);
						TextureIndex++;
					}
					break;
				case AttachmentType::StencilAndDepth:
					if (m_Attachments[i].isRenderable)
					{
						glCreateRenderbuffers(1, &m_Renderbuffers[RenderIndex]);
						glBindRenderbuffer(GL_RENDERBUFFER, m_Renderbuffers[RenderIndex]);
						glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, m_Attachments[i].Width, m_Attachments[i].Height);
						glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_Renderbuffers[RenderIndex]);
						RenderIndex++;
					}
					else
					{
						glCreateTextures(GL_TEXTURE_2D, 1, &m_Textures[TextureIndex]);
						glBindTexture(GL_TEXTURE_2D, m_Textures[TextureIndex]);
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Attachments[i].Width, m_Attachments[i].Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_Textures[TextureIndex], 0);
						TextureIndex++;
					}
					break;
				default:
					LOGERROR("Not a valid type");
					break;
				}
			}

		}

		if (!(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE))
		{
			std::string msg = "Framebuffer failed, error code: " + std::to_string(glCheckFramebufferStatus(GL_FRAMEBUFFER));
			LOGERROR(msg.c_str());
		}


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void Framebuffer::UseFramebuffer()
	{
		FramebufferShader->Use();
		FramebufferVAO->Bind();
		FramebufferVBO->Bind();
	}


	Framebuffer::~Framebuffer()
	{
		glDeleteFramebuffers(1, &m_Framebuffer);

		for (int i = 0; i < TextureIndex; i++)
			glDeleteTextures(1, &m_Textures[i]);

		for (int i = 0; i < RenderIndex; i++)
			glDeleteRenderbuffers(1, &m_Renderbuffers[i]);
	}

}
