#include "pch.h"
#include "VertexArray.h"

namespace TooGoodEngine
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
	void OpenGLVertexArray::AttachBuffer(OpenGLBuffer* gBuffer)
	{
		glBindVertexArray(m_VertexArrayHandle);
		gBuffer->Bind();
	}
	void OpenGLVertexArray::AttachBuffers(const std::vector<OpenGLBuffer*>& buffers)
	{
		glBindVertexArray(m_VertexArrayHandle);
		for (auto buffer : buffers)
			buffer->Bind();
	}
	void OpenGLVertexArray::AttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalised, GLsizei stride, const void* start)
	{
		glBindVertexArray(m_VertexArrayHandle);

		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, size, type, normalised, stride, start);

	}
	void OpenGLVertexArray::AttachAttribPointers(const std::vector<AttributeType>& Attribs)
	{
		size_t stride = 0;
		
		constexpr size_t FloatSize = sizeof(float);

		/*Calculate stride*/
		for (AttributeType type : Attribs)
		{
			switch (type)
			{
			case AttributeType::FLOAT:			stride += FloatSize;	 break;
			case AttributeType::FLOAT_2:		stride += FloatSize * 2; break;
			case AttributeType::FLOAT_3:		stride += FloatSize * 3; break;
			case AttributeType::FLOAT_4:		stride += FloatSize * 4; break;
			case AttributeType::MAT_4:			stride += FloatSize * 16; break;
			default:
				break;
			}
		}

		glBindVertexArray(m_VertexArrayHandle);

		size_t index = 0;
		size_t CurrentStartingPosition = 0;

		/* enable attributes here */

		for (AttributeType type : Attribs)
		{	
			GLint GLsize = 0;
			GLenum GLtype = 0;

			switch (type)
			{
			case AttributeType::FLOAT:	   GLsize = 1;  GLtype = GL_FLOAT;  break;
			case AttributeType::FLOAT_2:   GLsize = 2;  GLtype = GL_FLOAT;  break;
			case AttributeType::FLOAT_3:   GLsize = 3;  GLtype = GL_FLOAT;  break;
			case AttributeType::FLOAT_4:   GLsize = 4;  GLtype = GL_FLOAT;  break;
			case AttributeType::MAT_4:	   GLsize = 16; GLtype = GL_FLOAT_MAT4; break;
			default:
				break;
			}

			if (GLtype == GL_FLOAT_MAT4)
			{
				GLsize = 4;
				GLtype = GL_FLOAT;

				for (size_t i = 0; i < 4; i++)
				{
					glEnableVertexAttribArray((GLuint)index);
					glVertexAttribPointer((GLuint)index, GLsize, GLtype, GL_FALSE, (GLsizei)stride, (void*)(CurrentStartingPosition));

					CurrentStartingPosition += GLsize * FloatSize;
					index++;
				}
			}
			else
			{
				glEnableVertexAttribArray((GLuint)index);
				glVertexAttribPointer((GLuint)index, GLsize, GLtype, GL_FALSE, (GLsizei)stride, (void*)(CurrentStartingPosition));

				CurrentStartingPosition += GLsize * FloatSize;
				index++;
			}

		

			
		}
	}
	AttribBufferReturnData OpenGLVertexArray::AttachAttribToBuffer(std::vector<BufferVertexAttributes>& Attribs)
	{
		size_t stride = 0;
		size_t totalStride = 0;

		std::vector<GLuint> Buffers;
		std::vector<GLintptr> Offsets;
		std::vector<GLsizei> Strides;

		constexpr size_t FloatSize = sizeof(float);

		/*Calculate stride*/

		for (BufferVertexAttributes& BufferAttribs : Attribs)
		{
			stride = 0;
			Buffers.push_back(BufferAttribs.buffer->Get());
			Offsets.push_back(0);

			for (AttributeType type : BufferAttribs.List)
			{
				switch (type)
				{
				case AttributeType::FLOAT:			stride += FloatSize;	 break;
				case AttributeType::FLOAT_2:		stride += FloatSize * 2; break;
				case AttributeType::FLOAT_3:		stride += FloatSize * 3; break;
				case AttributeType::FLOAT_4:		stride += FloatSize * 4; break;
				case AttributeType::MAT_4:			stride += FloatSize * 16; break;
				default:
					break;
				}
			}

			Strides.push_back((GLuint)stride);
		}

		glBindVertexArray(m_VertexArrayHandle);

		size_t index = 0;
		size_t CurrentStartingPosition = 0;
		size_t CurrentBufferIndex = 0;

		for (BufferVertexAttributes& BufferAttribs : Attribs)
		{
			CurrentStartingPosition = 0;

			for (AttributeType type : BufferAttribs.List)
			{
				GLint GLsize = 0;
				GLenum GLtype = 0;

				switch (type)
				{
				case AttributeType::FLOAT:	   GLsize = 1;  GLtype = GL_FLOAT;  break;
				case AttributeType::FLOAT_2:   GLsize = 2;  GLtype = GL_FLOAT;  break;
				case AttributeType::FLOAT_3:   GLsize = 3;  GLtype = GL_FLOAT;  break;
				case AttributeType::FLOAT_4:   GLsize = 4;  GLtype = GL_FLOAT;  break;
				case AttributeType::MAT_4:	   GLsize = 16; GLtype = GL_FLOAT_MAT4; break;
				default:
					break;
				}

				if (GLtype == GL_FLOAT_MAT4)
				{
					GLsize = 4;
					GLtype = GL_FLOAT;

					for (size_t i = 0; i < 4; i++)
					{
						// Use DSA functions to set up vertex attributes
						glVertexArrayAttribFormat(m_VertexArrayHandle, (GLuint)index, GLsize, GLtype, GL_FALSE, (GLuint)CurrentStartingPosition);

						// Enable the vertex attribute and associate it with the VBO binding point
						glEnableVertexArrayAttrib(m_VertexArrayHandle, (GLuint)index);
						glVertexArrayAttribBinding(m_VertexArrayHandle, (GLuint)index, (GLuint)CurrentBufferIndex);

						// Specify the VBO binding point and stride
						glVertexArrayVertexBuffer(m_VertexArrayHandle, (GLuint)CurrentBufferIndex, Buffers[CurrentBufferIndex], (GLintptr)Offsets[CurrentBufferIndex], (GLsizei)Strides[CurrentBufferIndex]);

						if (BufferAttribs.Instanced)
							glVertexArrayBindingDivisor(m_VertexArrayHandle, (GLuint)CurrentBufferIndex, 1);

						CurrentStartingPosition += GLsize * FloatSize;
						index++;
					}
				}
				else
				{
					// Use DSA functions to set up vertex attributes
					glVertexArrayAttribFormat(m_VertexArrayHandle, (GLuint)index, GLsize, GLtype, GL_FALSE, (GLuint)CurrentStartingPosition);

					// Enable the vertex attribute and associate it with the VBO binding point
					glEnableVertexArrayAttrib(m_VertexArrayHandle, (GLuint)index);
					glVertexArrayAttribBinding(m_VertexArrayHandle, (GLuint)index, (GLuint)CurrentBufferIndex);

					// Specify the VBO binding point and stride
					glVertexArrayVertexBuffer(m_VertexArrayHandle, (GLuint)CurrentBufferIndex, Buffers[CurrentBufferIndex], (GLintptr)Offsets[CurrentBufferIndex], (GLsizei)Strides[CurrentBufferIndex]);

					if (BufferAttribs.Instanced)
						glVertexArrayBindingDivisor(m_VertexArrayHandle, (GLuint)CurrentBufferIndex, 1);



					CurrentStartingPosition += GLsize * FloatSize;
					index++;
				}

				
			}
			CurrentBufferIndex++;
		}


		return { Offsets, Strides };
	}
	void OpenGLVertexArray::AttachVertexBuffer(OpenGLBuffer& buffer, uint32_t BindingIndex, size_t Offset, size_t Stride)
	{
		glVertexArrayVertexBuffer(m_VertexArrayHandle, BindingIndex, buffer.Get(), (GLintptr)Offset, (GLsizei)Stride);
	}
	void OpenGLVertexArray::AttachIndexBuffer(OpenGLBuffer& buffer)
	{
		glVertexArrayElementBuffer(m_VertexArrayHandle, buffer.Get());
	}
}