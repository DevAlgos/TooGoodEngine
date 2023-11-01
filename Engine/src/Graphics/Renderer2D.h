#pragma once

#include "VertexArray.h"
#include "GraphicsUtils.h"
#include "Buffers.h"
#include <Utils/Camera.h>
#include <memory>


namespace Graphics
{
	struct RendererData2D
	{
		static const int32_t MaxQuads = 1000;
		static const int32_t MaxIndicies = MaxQuads * 6;
		static const int32_t MaxVertices = MaxQuads * 4;

		static const size_t MaxLightSources = 6;

		OrthoGraphicCamera Camera;

		Vertex* Buffer = nullptr;
		Vertex* BufferIndex = nullptr;

		std::unique_ptr<Framebuffer> TestFrameBuffer;

		std::unique_ptr<BufferObject> VertexBuffer;
		std::unique_ptr<BufferObject> IndexBuffer;
		std::unique_ptr<BufferObject> UniformBuffer; //Material Buffer
		std::unique_ptr<BufferObject> LightUniformBuffer; //Light Buffer;
		std::unique_ptr<VertexArrayObject> VertexArray;

		std::unique_ptr<Shader> DefaultShader;

		uint32_t Indicies[MaxQuads * 6];

		std::vector<uint32_t> TextureSlots;
		uint8_t CurrentTextureSlot;
		GLint MaxTextureSlots;

		std::vector<Material> AllMaterials;
		int32_t MaxMaterialSlots = 32;
		uint32_t CurrentMaterialSlot = 0;

		uint32_t DefaultTexture;
		uint32_t IndexCount;

		size_t MaterialStride = sizeof(Material);
		size_t LightStride = sizeof(LightSource);

		std::array<LightSource, MaxLightSources> LightSources;
		size_t CurrentLightSlot = 0;
	};

	class Renderer2D
	{
	public:
		Renderer2D() = delete;
		~Renderer2D() = default;

		static void ClearColor(const glm::vec3& Color);
		static void ClearDepth(const glm::vec3& Color);
		static void Clear();
		static void Init();

		static void BeginScene(OrthoGraphicCamera& Camera);
		static void BeginScene();

		static void PushQuad(const glm::vec3& Position, float size, uint32_t ID, const glm::mat4& ModelMatrix);
		static void PushQuad(const glm::vec3& Position, float size, const glm::vec4& color, const glm::mat4& ModelMatrix);
		static void PushSprite(const glm::vec3& Position, float size, uint32_t ID, SpriteSheet sheet, const glm::mat4& ModelMatrix);

		static void PushQuad(const glm::vec3& Position, float size, uint32_t ID, Material& material, const glm::mat4& ModelMatrix);
		static void PushQuad(const glm::vec3& Position, float size, const glm::vec4& color, Material& material, const glm::mat4& ModelMatrix);
		static void PushSprite(const glm::vec3& Position, float size, uint32_t ID, SpriteSheet sheet, Material& material, const glm::mat4& ModelMatrix);

		static void PushLight(const LightSource& light);

		static void SetUniformVec3(const glm::vec3& data, const char* name);

		static void Draw();

		static void EndScene();
		static void ShutDown();

		static void FlushScene();

	private:


	};

}