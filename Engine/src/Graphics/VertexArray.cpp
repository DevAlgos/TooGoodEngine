#include <pch.h>
#include "VertexArray.h"

namespace Graphics
{
	VertexArrayObject::VertexArrayObject()
	{
	}
	VertexArrayObject::~VertexArrayObject()
	{
		glDeleteVertexArrays(1, &m_VAO);
	}
	void VertexArrayObject::AttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalised, GLsizei stride, const void* start)
	{
		Bind();

		glVertexAttribPointer(index, size, type, normalised, stride, start);
		glEnableVertexAttribArray(index);

		Unbind();

	}
}