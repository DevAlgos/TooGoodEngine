#include <pch.h>
#include "VertexArray.h"

namespace TGE
{
	VertexArrayObject::VertexArrayObject()
		: m_VAO(0)
	{
	}
	VertexArrayObject::~VertexArrayObject()
	{
		glDeleteVertexArrays(1, &m_VAO);
	}
	void VertexArrayObject::AttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalised, GLsizei stride, const void* start)
	{
		Bind();

		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, size, type, normalised, stride, start);

	}
}