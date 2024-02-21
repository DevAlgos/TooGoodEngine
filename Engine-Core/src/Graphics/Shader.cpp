#include <pch.h>
#include "Shader.h"

namespace TooGoodEngine {
	Shader::Shader(std::map<GLenum, std::string_view> shaders)
	{
		m_Program = glCreateProgram();

		for (auto& [ShaderType, ShaderLocation] : shaders)
		{
			std::string contents = LoadShaderFromFile(ShaderLocation.data());
			const char* data = contents.c_str();

			uint32_t Shader = glCreateShader(ShaderType);
			glShaderSource(Shader, 1, &data, nullptr);
			glCompileShader(Shader);
			glAttachShader(m_Program, Shader);

			CheckShaderError(GL_COMPILE_STATUS, Shader);

			glDeleteShader(Shader);
		}

		glLinkProgram(m_Program);
		CheckProgramError(GL_LINK_STATUS);
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
	std::unique_ptr<Shader> Shader::Generate(std::map<GLenum, std::string_view> shaders)
	{
		return std::make_unique<Shader>(shaders);
	}
	std::shared_ptr<Shader> Shader::GenerateShared(std::map<GLenum, std::string_view> shaders)
	{
		return std::make_shared<Shader>(shaders);
	}
	void Shader::SetUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_Program, name.c_str());
		if (location != -1)
			glUniform1i(location, value);
		else 
			TGE_LOG_ERROR("Uniform Location Failed At: ", name);
	}
	void Shader::SetUniformIntV(const std::string& name, int value[], int numb)
	{
		GLint location = glGetUniformLocation(m_Program, name.c_str());
		if (location != -1)
			glUniform1iv(location, numb, value);
		else 
			TGE_LOG_ERROR("Uniform Location Failed At: ", name);
	}
	void Shader::SetUniformFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_Program, name.c_str());
		if (location != -1)
			glUniform1f(location, value);
		else
			TGE_LOG_ERROR("Uniform Location Failed At: ", name);

	}
	void Shader::SetUniformFloat2(const std::string& name, float v0, float v1)
	{
		GLint location = glGetUniformLocation(m_Program, name.c_str());
		if (location != -1)
			glUniform2f(location, v0, v1);
		else
			TGE_LOG_ERROR("Uniform Location Failed At: ", name);

	}
	void Shader::SetUniformFloat3(const std::string& name, float v0, float v1, float v2)
	{
		GLint location = glGetUniformLocation(m_Program, name.c_str());
		if (location != -1)
			glUniform3f(location, v0, v1, v2);
		else
			TGE_LOG_ERROR("Uniform Location Failed At: ", name);
	}
	void Shader::SetUniformFloat4(const std::string& name, float v0, float v1, float v2, float v3)
	{
		GLint location = glGetUniformLocation(m_Program, name.c_str());
		if (location != -1)
			glUniform4f(location, v0, v1, v2, v3);
		else
			TGE_LOG_ERROR("Uniform Location Failed At: ", name);
	}
	void Shader::SetUniformFloat3V(const std::string& name, glm::vec3 floats, int numb)
	{
		GLint location = glGetUniformLocation(m_Program, name.c_str());
		if (location != -1)
			glUniform3fv(location, numb, glm::value_ptr(floats));
		else
			TGE_LOG_ERROR("Uniform Location Failed At: ", name);
	}
	void Shader::SetUniformMat4(const std::string& name, const glm::mat4& matrix)
	{

		GLint location = glGetUniformLocation(m_Program, name.c_str());
		if (location != -1)
			glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
		else
			TGE_LOG_ERROR("Uniform Location Failed At: ", name);
	}
	void Shader::SetUniformVec3(const std::string& name, const glm::vec3& Data, int Numb)
	{
		GLint location = glGetUniformLocation(m_Program, name.c_str());
		if (location != -1)
			glUniform3fv(location, Numb, glm::value_ptr(Data));
		else
			TGE_LOG_ERROR("Uniform Location Failed At: ", name);
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

			TGE_LOG_ERROR(infoLog);
		}
	}
	void Shader::CheckShaderError(GLenum status, uint32_t shader)
	{
		GLint success;
		glGetShaderiv(shader, status, &success);
		if (!success) {
			char infoLog[512];
			glGetShaderInfoLog(shader, 512, nullptr, infoLog);

			TGE_LOG_ERROR(infoLog);
		}
	}
	std::string Shader::LoadShaderFromFile(const char* fileLocation)
	{
		FILE* File = nullptr;
		fopen_s(&File, fileLocation, "r");

		if (!File)
		{
			TGE_CLIENT_ERROR("File couldn't open ", fileLocation);
			return "no shader";
		}

		std::string FileContents;

		char buffer[100]{};

		while (fgets(buffer, sizeof(buffer), File) != nullptr)
			FileContents += buffer;
		

		fclose(File);
		return FileContents;
	}

}

