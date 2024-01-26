#pragma once

#include "Shader.h"

#include <glad/glad.h>
#include <map>
#include <variant>

namespace TooGoodEngine
{
	static int s_GlobalID = 0;

	template<class Type>
	constexpr size_t FindSize(Type* data)
	{
		return sizeof(data) / sizeof(data[0]);
	}

	struct BufferData
	{
		const void* data;
		size_t VertexSize;
		GLenum DrawType;

	};

	struct DynamicData
	{
		const void* data; //Not needed for BindRange() however is needed for PushData
		size_t VertexSize;
		GLintptr Offset = 0;
		GLuint index = 0;
	};

	enum class BufferType { VertexBuffer = 0, IndexBuffer, UniformBuffer, ShaderStorageBuffer };

	class OpenGLBuffer
	{
	public:
		OpenGLBuffer(const BufferType& t, const BufferData& BufferData);

		void* Map();
		void  UnMap();

		void* MapBufferRange();

		static std::unique_ptr<OpenGLBuffer> Generate(const BufferType& t, const BufferData& BufferData);
		static std::shared_ptr<OpenGLBuffer> GenerateShared(const BufferType& t, const BufferData& BufferData);

		void Resize(uint32_t size);

		void Bind();
		void PushData(const DynamicData& data);
		void BindRange(const DynamicData& data);
		void BindBase(GLuint index);

		inline const uint32_t Get() const { return m_Buffer; }

		~OpenGLBuffer();

	private:
		uint32_t m_Buffer;
		size_t m_Size = 0;

		GLbitfield masks = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT; //TODO: will make it changable later


		GLenum m_Type;
		BufferData m_Data;
	};

	enum class AttachmentType { Color = 0, Stencil, Depth, Depth24Stencil8, Depth32Stencil8 };

	struct RenderBufferData
	{
		TextureFormat InternalFormat = TextureFormat::RGBA8;
		uint32_t width;
		uint32_t height;
	};

	class RenderBuffer
	{
	public:
		RenderBuffer(const RenderBufferData& data);
		~RenderBuffer();

		const void Bind() const { glBindRenderbuffer(GL_FRAMEBUFFER, m_RenderBufferHandle); }
		
		const inline uint32_t Get() const { return m_RenderBufferHandle; }

		static std::unique_ptr<RenderBuffer> Generate(const RenderBufferData& data);
		static std::shared_ptr<RenderBuffer> GenerateShared(const RenderBufferData& data);
	private:
		uint32_t m_RenderBufferHandle;
	};

	template<class Type>
	concept IsTextureOrRenderBuffer =
		std::is_same_v<Type, Texture> || std::is_same_v<Type, RenderBuffer>;

	using TextureOrRenderBuffer = std::variant<std::shared_ptr<Texture>, std::shared_ptr<RenderBuffer>>;

	struct Attachment
	{
		AttachmentType Type;
		int Width, Height;

		Attachment(AttachmentType type, int width, int height)
			: Type(type), Width(width), Height(height), m_ID(s_GlobalID++)
		{
		}

		const size_t GetID() const { return m_ID; }

		bool operator>(const Attachment& other) const
		{
			return m_ID > other.m_ID;
		}

		bool operator<(const Attachment& other) const
		{
			return m_ID < other.m_ID;
		}

	private:
		size_t m_ID;
	};	

	struct FramebufferData
	{
		std::map<Attachment, TextureOrRenderBuffer> AttachmentList;
	};

	class Framebuffer
	{
	public:
		Framebuffer(const FramebufferData& data);
		~Framebuffer();

		inline const void Bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferHandle); }
		inline const void UnBind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }


		inline const uint32_t Get() const { return m_FramebufferHandle; }

		static std::unique_ptr<Framebuffer> Generate(const FramebufferData& data);
		static std::shared_ptr<Framebuffer> GenerateShared(const FramebufferData& data);
	private:
		uint32_t m_FramebufferHandle;
	};
}