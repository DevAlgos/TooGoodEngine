#pragma once

#include <glad/glad.h>
#include "Shader.h"
#include "VertexArray.h"
#include <map>

namespace Graphics
{
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


	class BufferObject
	{
	public:
		enum class BufferType { VertexBuffer = 0, IndexBuffer, UniformBuffer, ShaderStorageBuffer };

	public:
		BufferObject(const BufferType& t, const BufferData& BufferData);

		void Create();
		void Bind();
		void PushData(const DynamicData& data);
		void BindRange(const DynamicData& data);
		void BindBase(GLuint index);

		~BufferObject();

	private:
		uint32_t m_Buffer;

		GLenum m_Type;
		BufferData m_Data;
	};


	enum class AttachmentType { Color = 0, Stencil, Depth, StencilAndDepth };

	struct Attachment
	{
		AttachmentType Type;
		bool isRenderable; //Render Buffer if true, Texture if false
		int Width, Height;
	};


	class Framebuffer
	{
	public:

	public:
		Framebuffer(std::vector<Attachment> Attachments, std::map<GLenum, const char*> ShaderList);
		~Framebuffer();

		inline void Bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer); }
		inline void Unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

		inline void BindTexture(int Unit) const { glBindTextureUnit(Unit, m_Textures[Unit]); }
		inline void BindRenderBuffer(int Index) const { glBindRenderbuffer(GL_RENDERBUFFER, m_Renderbuffers[Index]); }
		
		void UseFramebuffer();
		inline void Draw() { glDrawArrays(GL_TRIANGLES, 0, 6); }

		inline uint32_t GetRenderBuffer(size_t index) const { return m_Renderbuffers[index]; }
		inline uint32_t GetTexture(size_t index) const { return m_Textures[index]; }

	private:
		void GenerateAttachment(GLenum AttachmentType, GLenum InternalFormat, 
								 bool isRenderable,
								 size_t Index);
	private:
		uint32_t m_Framebuffer;
		size_t NumberOfRenderTargets;

		std::array<Attachment, 34> m_Attachments; //32 Color Attachments, 1 Stencil and Depth/1 StencilAndDepth attachment
		size_t nColorAttachments = 0;

		std::vector<uint32_t> m_Renderbuffers;
		size_t RenderIndex = 0;

		std::vector<uint32_t> m_Textures;
		size_t TextureIndex = 0;

		std::unique_ptr<Shader> FramebufferShader;
		std::unique_ptr<VertexArrayObject> FramebufferVAO;
		std::unique_ptr<BufferObject> FramebufferVBO;
	};
}