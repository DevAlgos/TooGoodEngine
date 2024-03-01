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

namespace TooGoodEngine 
{
	OpenGLBuffer::OpenGLBuffer(const BufferType& Type, const BufferData& BufferData)
		: m_Data(BufferData), m_Buffer(0)
	{
		switch (Type)
		{
		case TooGoodEngine::BufferType::VertexBuffer:          m_Type = GL_ARRAY_BUFFER; break;	
		case TooGoodEngine::BufferType::IndexBuffer:		   m_Type = GL_ELEMENT_ARRAY_BUFFER; break;
		case TooGoodEngine::BufferType::UniformBuffer:	       m_Type = GL_UNIFORM_BUFFER; break;
		case TooGoodEngine::BufferType::ShaderStorageBuffer:   m_Type = GL_SHADER_STORAGE_BUFFER; break;
		default:								               m_Type = GL_ARRAY_BUFFER; break;
		}

		m_Size = m_Data.VertexSize;

		glCreateBuffers(1, &m_Buffer);
		glNamedBufferStorage(m_Buffer, m_Data.VertexSize, m_Data.data, masks);

		
	}
	void* OpenGLBuffer::Map()
	{
		return glMapNamedBuffer(m_Buffer, GL_WRITE_ONLY);
	}
	void OpenGLBuffer::UnMap()
	{
		glUnmapNamedBuffer(m_Buffer);
	}
	void* OpenGLBuffer::MapBufferRange()
	{
		return glMapNamedBufferRange(m_Buffer, 0, m_Size, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	}
	std::unique_ptr<OpenGLBuffer> OpenGLBuffer::Generate(const BufferType& t, const BufferData& BufferData)
	{
		return std::make_unique<OpenGLBuffer>(t, BufferData);
	}
	std::shared_ptr<OpenGLBuffer> OpenGLBuffer::GenerateShared(const BufferType& t, const BufferData& BufferData)
	{
		return std::make_shared<OpenGLBuffer>(t, BufferData);
	}
	void OpenGLBuffer::Resize(uint32_t size)
	{
		uint32_t NewBuffer;

		glCreateBuffers(1, &NewBuffer);
		glNamedBufferStorage(NewBuffer, size, nullptr, masks);  // Allocate memory, but don't copy data yet
		glCopyNamedBufferSubData(m_Buffer, NewBuffer, 0, 0, (GLsizeiptr)std::min((size_t)size, m_Size));

		glDeleteBuffers(1, &m_Buffer);

		m_Size = size;
		m_Buffer = NewBuffer;
	}
	OpenGLBuffer::~OpenGLBuffer()
	{
		glDeleteBuffers(1, &m_Buffer);
	}
	void OpenGLBuffer::BindRange(const DynamicData& data)
	{
		glBindBufferRange(m_Type, data.index, m_Buffer, data.Offset, data.VertexSize);
	}
	void OpenGLBuffer::BindBase(GLuint index)
	{
		glBindBufferBase(m_Type, index, m_Buffer);
	}
	void OpenGLBuffer::PushData(const DynamicData& data)
	{
		if (!data.data)
		{
			TGE_LOG_ERROR("Data is nullptr when pushing data!");
			TGE_CLIENT_ERROR("Data is nullptr when pushing data!");
			return;
		}

		glNamedBufferSubData(m_Buffer, data.Offset, data.VertexSize, data.data);
	}
	void OpenGLBuffer::Bind()
	{
		glBindBuffer(m_Type, m_Buffer);
	}

	
	RenderBuffer::RenderBuffer(const RenderBufferData& data)
		: m_RenderBufferHandle(std::numeric_limits<uint32_t>::max())
	{
		glCreateRenderbuffers(1, &m_RenderBufferHandle);
		//glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBufferHandle);

		//TODO: Test to make sure new DSA functions work

		GLenum GLInternalFormat = 0;

		switch (data.InternalFormat)
		{
		case TextureFormat::RGBA8:	 GLInternalFormat = GL_RGBA8;   break;
		case TextureFormat::RGBA16F: GLInternalFormat = GL_RGBA16F; break;
		case TextureFormat::RGBA32F: GLInternalFormat = GL_RGBA32F; break;
		default:					 GLInternalFormat = GL_RGBA;    break;
		}

		//glRenderbufferStorage(GL_RENDERBUFFER, GLInternalFormat, data.width, data.height);
		glNamedRenderbufferStorage(m_RenderBufferHandle, GLInternalFormat, data.width, data.height);
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
				TGE_CLIENT_WARN("Not a valid attachment type, skipping attachment ", AttachmentData.GetID());
				continue;
				break;
			}

			if (std::holds_alternative<std::shared_ptr<Texture>>(Medium))
			{
				auto& TextureMedium = std::get<std::shared_ptr<Texture>>(Medium);
				glNamedFramebufferTexture(m_FramebufferHandle, GLAttachType, TextureMedium->Get(), 0);
			
			}
			else
			{
				auto& RenderBufferMedium = std::get<std::shared_ptr<RenderBuffer>>(Medium);
				glNamedFramebufferRenderbuffer(m_FramebufferHandle, GLAttachType, GL_RENDERBUFFER,
					RenderBufferMedium->Get());
			}
		}

		GLenum status = glCheckNamedFramebufferStatus(m_FramebufferHandle, GL_FRAMEBUFFER);
		TGE_ASSERT(status == GL_FRAMEBUFFER_COMPLETE, "framebuffer incomplete with code ", status);
		

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
