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
	std::unique_ptr<VertexArrayObject> VertexArrayObject::Generate()
	{
		return std::make_unique<VertexArrayObject>();
	}
	std::shared_ptr<VertexArrayObject> VertexArrayObject::GenerateShared()
	{
		return std::make_shared<VertexArrayObject>();
	}
	void VertexArrayObject::AttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalised, GLsizei stride, const void* start)
	{
		Bind();

		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, size, type, normalised, stride, start);

	}
}