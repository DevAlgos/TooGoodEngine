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
		static const uint32_t MaxQuads = 1000;

		static const uint32_t MaxIndicies = MaxQuads * 6;
		static const uint32_t MaxVertices = MaxQuads * 4;

		static const uint32_t MaxLightSources = 6;

		OrthoGraphicCamera Camera;

		Vertex* Buffer = nullptr;
		Vertex* BufferIndex = nullptr;

		CircleVertex* CircleBuffer = nullptr;
		CircleVertex* CircleBufferIndex = nullptr;

		std::unique_ptr<BufferObject>       VertexBuffer;
		std::unique_ptr<BufferObject>       IndexBuffer;
		std::unique_ptr<BufferObject>       UniformBuffer; //Material Buffer
		std::unique_ptr<BufferObject>       LightUniformBuffer; //Light Buffer;
		std::unique_ptr<VertexArrayObject>  VertexArray;
		std::unique_ptr<Shader>				DefaultShader;


		std::unique_ptr<BufferObject>			CircleVertexBuffer;
		std::unique_ptr<VertexArrayObject>		CircleVertexArray;
		std::unique_ptr<Shader>					CircleShader;


		std::unique_ptr<BufferObject>			ShaderStorage;
		std::vector<glm::vec4>					ObjectAttributes;

		uint32_t Indicies[MaxQuads * 6];

		std::vector<uint32_t> TextureSlots;
		uint32_t CurrentTextureSlot;
		GLint MaxTextureSlots;

		std::vector<Material> AllMaterials;
		uint32_t MaxMaterialSlots = 32;
		uint32_t CurrentMaterialSlot = 0;

		uint32_t DefaultTexture;
		uint32_t IndexCount;
		uint32_t CircleIndexCount;

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

		static void PushQuad(const glm::vec3& Position, const glm::vec2& size, float Rotation, uint32_t ID);
		static void PushQuad(const glm::vec3& Position, const glm::vec2& size, float Rotation, const glm::vec4& color);
		static void PushSprite(const glm::vec3& Position, const glm::vec2& size, float Rotation, uint32_t ID, SpriteSheet sheet);

		static void PushQuad(const glm::vec3& Position, const glm::vec2& size, float Rotation, uint32_t ID, Material& material);
		static void PushQuad(const glm::vec3& Position, const glm::vec2& size, float Rotation, const glm::vec4& color, Material& material);
		static void PushSprite(const glm::vec3& Position, const glm::vec2& size, float Rotation, uint32_t ID, SpriteSheet sheet, Material& material);

		static void PushCircle(const glm::vec3& Position, const glm::vec2& size, float Rotation, float Thickness, const glm::vec4& color);

		static void PushLight(const LightSource& light);

		static void SetUniformVec3(const glm::vec3& data, const char* name);

		static void DrawQuad();
		static void DrawCircle();

		static void EndScene();
		static void ShutDown();

		static void FlushScene();

	private:


	};

}