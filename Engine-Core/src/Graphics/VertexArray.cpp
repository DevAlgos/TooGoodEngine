#include <pch.h>
#include "VertexArray.h"

namespace tge
{
	OpenGLVertexArray::OpenGLVertexArray()
		: m_VertexArrayHandle(0)
	{
		glCreateVertexArrays(1, &m_VertexArrayHandle);
	}
	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_VertexArrayHandle);
	}
	std::unique_ptr<OpenGLVertexArray> OpenGLVertexArray::Generate()
	{
		return std::make_unique<OpenGLVertexArray>();
	}
	std::shared_ptr<OpenGLVertexArray> OpenGLVertexArray::GenerateShared()
	{
		return std::make_shared<OpenGLVertexArray>();
	}
	void OpenGLVertexArray::AttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalised, GLsizei stride, const void* start)
	{
		Bind();

		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, size, type, normalised, stride, start);

	}
	void OpenGLVertexArray::AttachAttribPointers(const std::vector<AttributeType>& Attribs)
	{
		size_t stride = 0;
		
		constexpr size_t FloatSize = sizeof(float);

		for (AttributeType type : Attribs)
		{
			switch (type)
			{
			case tge::AttributeType::FLOAT:			stride += FloatSize;	 break;
			case tge::AttributeType::FLOAT_2:		stride += FloatSize * 2; break;
			case tge::AttributeType::FLOAT_3:		stride += FloatSize * 3; break;
			case tge::AttributeType::FLOAT_4:		stride += FloatSize * 4; break;
			case tge::AttributeType::MAT_4:			stride += FloatSize * 16; break;
			default:
				break;
			}
		}

		glBindVertexArray(m_VertexArrayHandle);

		size_t index = 0;
		size_t CurrentStartingPosition = 0;

		for (AttributeType type : Attribs)
		{	
			GLint GLsize = 0;
			GLenum GLtype = 0;

			switch (type)
			{
			case tge::AttributeType::FLOAT:	   GLsize = 1;  GLtype = GL_FLOAT;  break;
			case tge::AttributeType::FLOAT_2:  GLsize = 2;  GLtype = GL_FLOAT;  break;
			case tge::AttributeType::FLOAT_3:  GLsize = 3;  GLtype = GL_FLOAT;  break;
			case tge::AttributeType::FLOAT_4:  GLsize = 4;  GLtype = GL_FLOAT;  break;
			case tge::AttributeType::MAT_4:	   GLsize = 16; GLtype = GL_FLOAT_MAT4; break;
			default:
				break;
			}

			if (GLtype == GL_FLOAT_MAT4)
			{
				GLsize = 4;
				GLtype = GL_FLOAT;

				for (size_t i = 0; i < 4; i++)
				{
					glEnableVertexAttribArray(index);
					glVertexAttribPointer(index, GLsize, GLtype, GL_FALSE, stride, (void*)(CurrentStartingPosition));

					CurrentStartingPosition += GLsize * FloatSize;
					index++;
				}
			}
			else
			{
				glEnableVertexAttribArray(index);
				glVertexAttribPointer(index, GLsize, GLtype, GL_FALSE, stride, (void*)(CurrentStartingPosition));

				CurrentStartingPosition += GLsize * FloatSize;
				index++;
			}

		

			
		}
	}
}