#pragma once

namespace tge {

	class Shader
	{
	public:
		Shader(std::map<GLenum, std::string_view> shaders);
		Shader(const Shader& shader);
		Shader();
		~Shader();

		static std::unique_ptr<Shader> Generate(std::map<GLenum, std::string_view> shaders);
		static std::shared_ptr<Shader> GenerateShared(std::map<GLenum, std::string_view> shaders);


		inline void Use() { glUseProgram(m_Program); }
		const inline uint32_t Get() const { return m_Program; }
		const inline void Compute(int xChunk, int yChunk, int zChunk) const
		{
			glDispatchCompute(xChunk, yChunk, zChunk);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
		}

		void SetUniformInt(const std::string& name, int value);
		void SetUniformIntV(const std::string& name, int value[], int numb);

		void SetUniformFloat(const std::string& name, float value);
		void SetUniformFloat2(const std::string& name, float v0, float v1);
		void SetUniformFloat3(const std::string& name, float v0, float v1, float v2);
		void SetUniformFloat3V(const std::string& name, glm::vec3 floats, int numb);
		void SetUniformFloat4(const std::string& name, float v0, float v1, float v2, float v3);
		void setUniformMat4(const std::string& name, const glm::mat4& matrix);

		Shader& operator=(const Shader& shader);

	private:
		std::string LoadShaderFromFile(const char* fileLocation);

		void CheckProgramError(GLenum status);
		void CheckShaderError(GLenum status, uint32_t shader);

		uint32_t m_Program;

	};
}