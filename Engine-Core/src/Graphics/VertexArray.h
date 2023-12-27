#pragma once

#include <glad/glad.h>
#include <iostream>

namespace TGE
{
	enum class AttributeType
	{
		FLOAT = 0, FLOAT_2, FLOAT_3, FLOAT_4,
		MAT_4
	};

	class VertexArrayObject
	{
	public:
		VertexArrayObject();
		~VertexArrayObject();

		static std::unique_ptr<VertexArrayObject> Generate();
		static std::shared_ptr<VertexArrayObject> GenerateShared();

		inline void Bind() { glBindVertexArray(m_VertexArrayHandle); }
		inline void Unbind() { glBindVertexArray(0); }
		inline uint32_t Get() { return m_VertexArrayHandle; }

		void AttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalised, GLsizei stride, const void* start);
		void AttachAttribPointers(const std::vector<AttributeType>& Attribs);

	private:
		uint32_t m_VertexArrayHandle;
	};
}