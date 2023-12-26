#pragma once

#include <glad/glad.h>
#include <iostream>

namespace TGE
{
	class VertexArrayObject
	{
	public:
		VertexArrayObject();
		~VertexArrayObject();

		static std::unique_ptr<VertexArrayObject> Generate();
		static std::shared_ptr<VertexArrayObject> GenerateShared();

		inline void Bind() { glBindVertexArray(m_VAO); }
		inline void Unbind() { glBindVertexArray(0); }
		inline void Create() { glCreateVertexArrays(1, &m_VAO); }
		inline uint32_t Get() { return m_VAO; }

		void AttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalised, GLsizei stride, const void* start);

	private:
		uint32_t m_VAO;
	};
}