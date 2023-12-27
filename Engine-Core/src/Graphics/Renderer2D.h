#pragma once

#include "VertexArray.h"
#include "GraphicsUtils.h"
#include "Buffers.h"
#include <Utils/Camera.h>
#include <memory>
#include "Texture.h"
#include <UI/UIManager.h>
#include <string_view>

namespace TGE
{
	struct RendererData2D
	{
#pragma region Constants
		static const uint32_t MaxQuads = 10000;
		static const uint32_t MaxUIComponents = 1000;

		static const uint32_t MaxIndicies = MaxQuads * 6;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxUIVertices = MaxUIComponents * 4;

		static const uint32_t MaxLightSources = 6;
#pragma endregion Constants

#pragma region UI
		UIManager UIManager;

		std::vector<uint32_t> UITextureSlots;
		uint32_t UITextureCount;

		std::unique_ptr<Shader> UIShaders;

		std::unique_ptr<VertexArrayObject> UIVao;
		std::unique_ptr<OpenGLBuffer> UIVbo; //we can use same index buffer

		UIVertex* UIBuffer = nullptr;
		UIVertex* UIBufferIndex = nullptr;

		uint32_t UIIndexCount = 0;
		uint32_t TestFont;

#pragma endregion UI

#pragma region RenderData
		OrthoGraphicCamera Camera;

		Vertex* Buffer = nullptr;
		Vertex* BufferIndex = nullptr;

		CircleVertex* CircleBuffer = nullptr;
		CircleVertex* CircleBufferIndex = nullptr;

#pragma endregion RenderData

#pragma region RenderBuffers
		std::unique_ptr<OpenGLBuffer>       VertexBuffer;
		std::unique_ptr<OpenGLBuffer>       IndexBuffer;
		std::unique_ptr<OpenGLBuffer>       UniformBuffer; //Material Buffer
		std::unique_ptr<OpenGLBuffer>       LightUniformBuffer; //Light Buffer;
		std::unique_ptr<VertexArrayObject>  VertexArray;
		std::unique_ptr<Shader>				DefaultShader;


		std::unique_ptr<OpenGLBuffer>			CircleVertexBuffer;
		std::unique_ptr<VertexArrayObject>		CircleVertexArray;
		std::unique_ptr<Shader>					CircleShader;


		std::unique_ptr<OpenGLBuffer>			ShaderStorage;
		std::vector<glm::vec4>					ObjectAttributes;

		uint32_t Indicies[MaxQuads * 6];
#pragma endregion RenderBuffers

#pragma region Slots
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
#pragma endregion Slots
	};

#pragma region Primary 2D Renderer
	class Renderer2D
	{
	public:
		Renderer2D() = delete;
		~Renderer2D() = default;

		static void ClearColor(const glm::vec3& Color);
		static void ClearDepth(const glm::vec3& Color);
		static void Clear();
		static void Init();

		static void LoadInFont(const std::string_view& FontLocation, uint32_t Index = 0);

		static void BeginScene(OrthoGraphicCamera& Camera);
		static void BeginScene();

		static void PushUIText(const std::string_view& Text, uint32_t Font, const glm::vec3& Position, const glm::vec2& size, float Rotation,  const glm::vec4& color);

		static void PushQuad(const glm::vec3& Position, const glm::vec2& size, float Rotation, uint32_t ID);
		static void PushQuad(const glm::vec3& Position, const glm::vec2& size, float Rotation, const glm::vec4& color);
		static void PushSprite(const glm::vec3& Position, const glm::vec2& size, float Rotation, uint32_t ID, SpriteSheet sheet);

		static void PushQuad(const glm::vec3& Position, const glm::vec2& size, float Rotation, uint32_t ID, Material& material);
		static void PushQuad(const glm::vec3& Position, const glm::vec2& size, float Rotation, const glm::vec4& color, Material& material);
		static void PushSprite(const glm::vec3& Position, const glm::vec2& size, float Rotation, uint32_t ID, SpriteSheet sheet, Material& material);

		static void PushCircle(const glm::vec3& Position, const glm::vec2& size, float Rotation, float Thickness, const glm::vec4& color);
		static void PushCircle(const glm::vec3& Position, const glm::vec2& size, float Rotation, float Thickness, const glm::vec4& color, Material& material);

		static void PushLight(const LightSource& light);

		static void SetUniformVec3(const glm::vec3& data, const char* name);

		static void DrawUI();
		static void DrawQuad();
		static void DrawCircle();

		static void EndScene();
		static void ShutDown();

		static void FlushScene();
	};

#pragma endregion Primary 2D Renderer

#pragma region Secondary 2D Raytracing Renderer

	//Base Reflectivity
	//Roughness
	//Albedo
	//Emmission

	struct Circle
	{
		glm::vec4 Position;
		glm::vec4 Radius;

		glm::vec4 Albedo;
		glm::vec4 Reflectivity;
		glm::vec4 Roughness;
		glm::vec4 Emission;
	};

	
	struct RaytracingData
	{
		//Default size
		int ImageWidth = 1920;
		int ImageHeight = 1080;
		std::vector<int> Widths;
		std::vector<int> Heights;

		std::vector<Circle> CircleData;

		std::unique_ptr<Shader> ComputeShaders;
		std::unique_ptr<Shader> Shaders;

		std::shared_ptr<Texture> RenderImage;

		std::unique_ptr<OpenGLBuffer> VertexBuffer;
		std::unique_ptr<OpenGLBuffer> IndexBuffer;
		std::unique_ptr<OpenGLBuffer> ShaderStorage;

		uint32_t* Data = nullptr;
		OrthoGraphicCamera OrthoCamera;
		Camera DebuggingCamera;

		uint32_t SampleRate = 2;

	};

	class Raytracing2D
	{
	public:
		Raytracing2D() = delete;
		~Raytracing2D() = delete;

		static void Init();
		static void ChangeSampleRate(uint32_t NewSampleRate);

		/*Just used for testing performance of different algorithms will remove when finished*/
		static void Test();

		static void PushCircle(const Circle& CircleData);

		static void Trace();

		static void Shutdown();

		static std::shared_ptr<Texture> GetRenderImage();

	};

#pragma endregion Secondary 2D Raytracing Renderer

}		