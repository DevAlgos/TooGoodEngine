#include <pch.h>
#include "Shader.h"

namespace Graphics {
	Shader::Shader(std::map<GLenum, const char*> shaders)
	{
		Create(shaders);
	}
	Shader::Shader(const Shader& shader)
	{
		m_Program = shader.m_Program;
	}
	Shader::Shader()
		: m_Program(0)
	{
	}
	Shader::~Shader()
	{
		glDeleteProgram(m_Program);
	}
	void Shader::Create(std::map<GLenum, const char*> shaders)
	{
		m_Program = glCreateProgram();

		for (auto& source : shaders)
		{

			std::string contents = LoadShaderFromFile(source.second);
			const char* data = contents.c_str();

			uint32_t Shader = glCreateShader(source.first);
			glShaderSource(Shader, 1, &data, nullptr);
			glCompileShader(Shader);
			glAttachShader(m_Program, Shader);

			CheckShaderError(GL_COMPILE_STATUS, Shader);

			glDeleteShader(Shader);

		}

		glLinkProgram(m_Program);
		CheckProgramError(GL_LINK_STATUS);
	}
	void Shader::SetUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_Program, name.c_str());
		if (location != -1)
			glUniform1i(location, value);
		else {
			std::string msg = name;
			std::string msg1 = "Unform Location Failed At: " + msg;

			LOGWARNING(msg1);

		}
	}
	void Shader::SetUniformIntV(const std::string& name, int value[], int numb)
	{
		GLint location = glGetUniformLocation(m_Program, name.c_str());
		if (location != -1)
			glUniform1iv(location, numb, value);
		else {
			std::string msg = name;
			std::string msg1 = "Unform Location Failed At: " + msg;

			LOGWARNING(msg1);

		}
	}
	void Shader::SetUniformFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_Program, name.c_str());
		if (location != -1)
			glUniform1f(location, value);
		else {
			std::string msg = name;
			std::string msg1 = "Unform Location Failed At: " + msg;

			LOGWARNING(msg1);

		}
	}
	void Shader::SetUniformFloat2(const std::string& name, float v0, float v1)
	{
		GLint location = glGetUniformLocation(m_Program, name.c_str());
		if (location != -1)
			glUniform2f(location, v0, v1);
		else {
			std::string msg = name;
			std::string msg1 = "Unform Location Failed At: " + msg;

			LOGWARNING(msg1);

		}
	}
	void Shader::SetUniformFloat3(const std::string& name, float v0, float v1, float v2)
	{
		GLint location = glGetUniformLocation(m_Program, name.c_str());
		if (location != -1)
			glUniform3f(location, v0, v1, v2);
		else {
			std::string msg = name;
			std::string msg1 = "Unform Location Failed At: " + msg;

			LOGWARNING(msg1);

		}
	}
	void Shader::SetUniformFloat4(const std::string& name, float v0, float v1, float v2, float v3)
	{
		GLint location = glGetUniformLocation(m_Program, name.c_str());
		if (location != -1)
			glUniform4f(location, v0, v1, v2, v3);
		else {
			std::string msg = name;
			std::string msg1 = "Unform Location Failed At: " + msg;

			LOGWARNING(msg1);

		}
	}
	void Shader::SetUniformFloat3V(const std::string& name, glm::vec3 floats, int numb)
	{
		GLint location = glGetUniformLocation(m_Program, name.c_str());
		if (location != -1)
			glUniform3fv(location, numb, glm::value_ptr(floats));
		else {
			std::string msg = name;
			std::string msg1 = "Unform Location Failed At: " + msg;

			LOGWARNING(msg1);

		}
	}
	void Shader::setUniformMat4(const std::string& name, const glm::mat4& matrix)
	{

		GLint location = glGetUniformLocation(m_Program, name.c_str());
		if (location != -1)
			glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
		else {
			std::string msg = name;
			std::string msg1 = "Unform Location Failed At: " + msg;

			LOGWARNING(msg1);

		}
	}
	Shader& Shader::operator=(const Shader& shader)
	{
		m_Program = shader.m_Program;
		return *this;
	}
	void Shader::CheckProgramError(GLenum status)
	{
		GLint success;
		glGetProgramiv(m_Program, status, &success);
		if (!success) {
			char infoLog[512];
			glGetProgramInfoLog(m_Program, 512, nullptr, infoLog);

			LOGERROR(infoLog);
		}
	}
	void Shader::CheckShaderError(GLenum status, uint32_t shader)
	{
		GLint success;
		glGetShaderiv(shader, status, &success);
		if (!success) {
			char infoLog[512];
			glGetShaderInfoLog(shader, 512, nullptr, infoLog);

			LOGERROR(infoLog);
		}
	}
	std::string Shader::LoadShaderFromFile(const char* fileLocation)
	{
		std::ifstream file(fileLocation);
		if (!file.is_open())
			LOGERROR("File Couldn't Open");

		std::string FileContents(
			(std::istreambuf_iterator<char>(file)),
			(std::istreambuf_iterator<char>())
		);


		return FileContents;
	}

}

