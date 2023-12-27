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

namespace TGE 
{

	OpenGLBuffer::OpenGLBuffer(const BufferType& Type, const BufferData& BufferData)
		: m_Data(BufferData), m_Buffer(std::numeric_limits<uint32_t>::max())
	{
		switch (Type)
		{
		case TGE::BufferType::VertexBuffer:        m_Type = GL_ARRAY_BUFFER; break;	
		case TGE::BufferType::IndexBuffer:		   m_Type = GL_ELEMENT_ARRAY_BUFFER; break;
		case TGE::BufferType::UniformBuffer:	   m_Type = GL_UNIFORM_BUFFER; break;
		case TGE::BufferType::ShaderStorageBuffer: m_Type = GL_SHADER_STORAGE_BUFFER; break;
		default:								   m_Type = GL_ARRAY_BUFFER; break;
		}

		glCreateBuffers(1, &m_Buffer);
		glBindBuffer(m_Type, m_Buffer);
		glBufferData(m_Type, m_Data.VertexSize, m_Data.data, m_Data.DrawType);
	}
	std::unique_ptr<OpenGLBuffer> OpenGLBuffer::Generate(const BufferType& t, const BufferData& BufferData)
	{
		return std::make_unique<OpenGLBuffer>(t, BufferData);
	}
	std::shared_ptr<OpenGLBuffer> OpenGLBuffer::GenerateShared(const BufferType& t, const BufferData& BufferData)
	{
		return std::make_shared<OpenGLBuffer>(t, BufferData);
	}
	OpenGLBuffer::~OpenGLBuffer()
	{
		glDeleteBuffers(1, &m_Buffer);
	}
	void OpenGLBuffer::BindRange(const DynamicData& data)
	{
		glBindBuffer(m_Type, m_Buffer);
		glBindBufferRange(m_Type, data.index, m_Buffer, data.Offset, data.VertexSize);
	}
	void OpenGLBuffer::BindBase(GLuint index)
	{
		glBindBuffer(m_Type, m_Buffer);
		glBindBufferBase(m_Type, index, m_Buffer);
	}
	void OpenGLBuffer::PushData(const DynamicData& data)
	{
		if (!data.data)
		{
			LOGERROR("Data is nullptr when pushing data!");
			LOG_CORE_ERROR("Data is nullptr when pushing data!");
			return;
		}

		glBindBuffer(m_Type, m_Buffer);
		glBufferSubData(m_Type, data.Offset, data.VertexSize, data.data);
	}
	void OpenGLBuffer::Bind()
	{
		glBindBuffer(m_Type, m_Buffer);
	}

	
	RenderBuffer::RenderBuffer(const RenderBufferData& data)
		: m_RenderBufferHandle(std::numeric_limits<uint32_t>::max())
	{
		glCreateRenderbuffers(1, &m_RenderBufferHandle);
		glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBufferHandle);

		GLenum GLInternalFormat = 0;

		switch (data.InternalFormat)
		{
		case TextureFormat::RGBA8:	 GLInternalFormat = GL_RGBA8;   break;
		case TextureFormat::RGBA16F: GLInternalFormat = GL_RGBA16F; break;
		case TextureFormat::RGBA32F: GLInternalFormat = GL_RGBA32F; break;
		default:					 GLInternalFormat = GL_RGBA;    break;
		}

		glRenderbufferStorage(GL_RENDERBUFFER, GLInternalFormat, data.width, data.height);
	}

	RenderBuffer::~RenderBuffer()
	{
		glDeleteRenderbuffers(1, &m_RenderBufferHandle);
	}

	std::unique_ptr<RenderBuffer> RenderBuffer::Generate(const RenderBufferData& data)
	{
		return std::make_unique<RenderBuffer>(data);
	}

	std::shared_ptr<RenderBuffer> RenderBuffer::GenerateShared(const RenderBufferData& data)
	{
		return std::make_shared<RenderBuffer>(data);
	}

	Framebuffer::Framebuffer(const FramebufferData& data)
		: m_FramebufferHandle(std::numeric_limits<uint32_t>::max())
	{
		glCreateFramebuffers(1, &m_FramebufferHandle);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferHandle);

		uint32_t ColorIndex = 0;

		for (auto& [AttachmentData, Medium] : data.AttachmentList)
		{
			GLenum GLAttachType = 0;
			switch (AttachmentData.Type)
			{
			case AttachmentType::Color:	  GLAttachType = GL_COLOR_ATTACHMENT0 + (GLenum)ColorIndex++; break;
			case AttachmentType::Stencil: GLAttachType = GL_STENCIL_ATTACHMENT; break;
			case AttachmentType::Depth:   GLAttachType = GL_DEPTH_ATTACHMENT; break;
			case AttachmentType::Depth24Stencil8: GLAttachType = GL_DEPTH24_STENCIL8; break;
			case AttachmentType::Depth32Stencil8: GLAttachType = GL_DEPTH32F_STENCIL8; break;
			default:
				std::string msg = "Not a valid attachment type, skipping attachment " + AttachmentData.GetID();
				LOG_CORE_WARNING(msg);
				continue;
				break;
			}

			if (std::holds_alternative<std::shared_ptr<Texture>>(Medium))
			{
				auto& TextureMedium = std::get<std::shared_ptr<Texture>>(Medium);
				glFramebufferTexture(GL_FRAMEBUFFER, GLAttachType, TextureMedium->Get(), 0);
			}
			else
			{
				auto& RenderBufferMedium = std::get<std::shared_ptr<RenderBuffer>>(Medium);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GLAttachType, GL_RENDERBUFFER,
					RenderBufferMedium->Get());
			}
		}
	}

	Framebuffer::~Framebuffer()
	{
		glDeleteFramebuffers(1, &m_FramebufferHandle);
	}

	std::unique_ptr<Framebuffer> Framebuffer::Generate(const FramebufferData& data)
	{
		return std::make_unique<Framebuffer>(data);
	}

	std::shared_ptr<Framebuffer> Framebuffer::GenerateShared(const FramebufferData& data)
	{
		return std::make_shared<Framebuffer>(data);
	}	
}
