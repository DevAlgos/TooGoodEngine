#pragma once

#include "Buffers.h"


#include <glad/glad.h>
#include <iostream>
#include <vector>

namespace TooGoodEngine
{
	enum class AttributeType
	{
		FLOAT = 0, FLOAT_2, FLOAT_3, FLOAT_4,
		MAT_4
	};

	struct BufferVertexAttributes
	{
		OpenGLBuffer* buffer = nullptr;
		std::vector<AttributeType> List;
		bool Instanced = false;
	};

	struct AttribBufferReturnData
	{
		std::vector<GLintptr> Offsets;
		std::vector<GLsizei>  Strides;
	};

	class OpenGLVertexArray
	{
	public:
		OpenGLVertexArray();
		~OpenGLVertexArray();

		static std::unique_ptr<OpenGLVertexArray> Generate();
		static std::shared_ptr<OpenGLVertexArray> GenerateShared();

		inline void Bind() { glBindVertexArray(m_VertexArrayHandle); }
		inline void Unbind() { glBindVertexArray(0); }
		inline const uint32_t Get() const { return m_VertexArrayHandle; }

		void AttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalised, GLsizei stride, const void* start);
		
		void AttachAttribPointers(const std::vector<AttributeType>& Attribs);


		/*
		* optionally use return data, this is incase you decide to delete a buffer bound to this array
		* and would like to reattch it to a point later
		*/
		AttribBufferReturnData AttachAttribToBuffer(std::vector<BufferVertexAttributes>& Attribs);
		void AttachVertexBuffer(OpenGLBuffer& buffer, uint32_t BindingIndex, size_t Offset, size_t Stride);
		void AttachIndexBuffer(OpenGLBuffer& buffer);

	private:
		uint32_t m_VertexArrayHandle;
	};
}