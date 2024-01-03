#pragma once

#include <glad/glad.h>
#include <iostream>

namespace TooGoodEngine
{
	enum class AttributeType
	{
		FLOAT = 0, FLOAT_2, FLOAT_3, FLOAT_4,
		MAT_4
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

	private:
		uint32_t m_VertexArrayHandle;
	};
}