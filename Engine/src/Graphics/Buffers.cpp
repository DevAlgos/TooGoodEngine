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
		: m_Data(BufferData)
	{
		switch (Type)
		{
		case Graphics::BufferObject::BufferType::VertexBuffer:
			m_Type = GL_ARRAY_BUFFER;
			break;
		case Graphics::BufferObject::BufferType::IndexBuffer:
			m_Type = GL_ELEMENT_ARRAY_BUFFER;
			break;
		case Graphics::BufferObject::BufferType::UniformBuffer:
			m_Type = GL_UNIFORM_BUFFER;
			break;
		case Graphics::BufferObject::BufferType::ShaderStorageBuffer:
			m_Type = GL_SHADER_STORAGE_BUFFER;
			break;
		default:
			m_Type = GL_ARRAY_BUFFER;
			break;
		}

		Create();
	}
	BufferObject::~BufferObject()
	{
		glDeleteBuffers(1, &m_Buffer);
	}
	void BufferObject::Create()
	{
		glCreateBuffers(1, &m_Buffer);
		glBindBuffer(m_Type, m_Buffer);
		glBufferData(m_Type, m_Data.VertexSize, m_Data.data, m_Data.DrawType);
	}
	void BufferObject::BindRange(const DynamicData& data)
	{
		if (data.data)
			LOGWARNING("Data initalized with values when not needed!");

		glBindBuffer(m_Type, m_Buffer);
		glBindBufferRange(m_Type, data.index, m_Buffer, data.Offset, data.VertexSize);
	}
	void BufferObject::BindBase(GLuint index)
	{
		glBindBuffer(m_Type, m_Buffer);
		glBindBufferBase(m_Type, index, m_Buffer);
	}
	void BufferObject::PushData(const DynamicData& data)
	{
		if (!data.data)
			LOGERROR("Data is nullptr!");

		glBindBuffer(m_Type, m_Buffer);
		glBufferSubData(m_Type, data.Offset, data.VertexSize, data.data);
	}
	void BufferObject::Bind()
	{
		glBindBuffer(m_Type, m_Buffer);
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
					GenerateAttachment( GL_COLOR_ATTACHMENT0 + (GLenum)nColorAttachments, GL_RGBA, 
						m_Attachments[i].isRenderable, i);

					nColorAttachments++;
					break;
				case AttachmentType::Stencil:
					GenerateAttachment(GL_STENCIL_ATTACHMENT, GL_RGBA, m_Attachments[i].isRenderable, i);

					break;
				case AttachmentType::Depth:
					GenerateAttachment(GL_DEPTH_ATTACHMENT, GL_RGBA, m_Attachments[i].isRenderable, i);

					break;
				case AttachmentType::StencilAndDepth:
					GenerateAttachment(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH32F_STENCIL8, m_Attachments[i].isRenderable, i);

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

	void Framebuffer::GenerateAttachment(GLenum AttachmentType, GLenum InternalFormat, bool isRenderable, 
										  size_t Index)
	{
		if (isRenderable)
		{
			glCreateRenderbuffers(1, &m_Renderbuffers[RenderIndex]);
			glBindRenderbuffer(GL_RENDERBUFFER, m_Renderbuffers[RenderIndex]);
			glRenderbufferStorage(GL_RENDERBUFFER, InternalFormat, m_Attachments[Index].Width, m_Attachments[Index].Height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, AttachmentType, GL_RENDERBUFFER, m_Renderbuffers[RenderIndex]);
			RenderIndex++;
		}
		else
		{
			glCreateTextures(GL_TEXTURE_2D, 1, &m_Textures[TextureIndex]);
			glBindTexture(GL_TEXTURE_2D, m_Textures[TextureIndex]);
			glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, m_Attachments[Index].Width, m_Attachments[Index].Height, 0, InternalFormat, GL_UNSIGNED_BYTE, nullptr);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glFramebufferTexture2D(GL_FRAMEBUFFER, AttachmentType, GL_TEXTURE_2D, m_Textures[TextureIndex], 0);
			TextureIndex++;
		}
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
