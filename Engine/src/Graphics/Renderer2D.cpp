#include <pch.h>

#include "Renderer2D.h"


namespace
{
	static Graphics::RendererData2D RenderData;
	static Graphics::RendererData2D RaytracingData;
}

namespace Graphics
{
	void Renderer2D::Init()
	{
		std::map<GLenum, const char*> ShaderList = {
			{GL_VERTEX_SHADER, "Engine/src/resources/shaders/Defaults/basicVertex.glsl"},
			{GL_FRAGMENT_SHADER, "Engine/src/resources/shaders/Defaults/basicFragment.glsl"},

		};

		RenderData.DefaultShader = std::make_unique<Shader>(ShaderList);

		RenderData.CurrentTextureSlot = 1;

		RenderData.Buffer = new Vertex[RenderData.MaxVertices];
		RenderData.BufferIndex = RenderData.Buffer;

		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &RenderData.MaxTextureSlots); //Queries the drivers for the maximum texture slot of the gpu
		RenderData.MaxTextureSlots = 32;

		RenderData.AllMaterials.resize(RenderData.MaxMaterialSlots);

		Material DefaultMaterial;
		DefaultMaterial.ambient = glm::vec3(0.2, 0.2, 0.2);
		DefaultMaterial.diffuse = glm::vec3(1.0, 1.0, 1.0);
		DefaultMaterial.specular = glm::vec3(1.0, 1.0, 1.0);
		DefaultMaterial.shininess = 32.0;


		for (int i = 0; i < (int)RenderData.MaxMaterialSlots - 1; i++)
			RenderData.AllMaterials[i] = DefaultMaterial;

		int* samplers = new int[RenderData.MaxTextureSlots];
		for (int i = 0; i < (int)RenderData.MaxTextureSlots; i++)
			samplers[i] = i;




		RenderData.DefaultShader->Use();
		RenderData.DefaultShader->SetUniformIntV("samplerTextures", samplers, RenderData.MaxTextureSlots);

		RenderData.TextureSlots.resize(RenderData.MaxTextureSlots);

		memset(RenderData.TextureSlots.data(), 0, RenderData.MaxTextureSlots);

		RenderData.VertexArray = std::make_unique<VertexArrayObject>();
		RenderData.VertexArray->Create();

		BufferData VertexData;
		VertexData.DrawType = GL_DYNAMIC_DRAW;
		VertexData.data = nullptr;
		VertexData.VertexSize = sizeof(Vertex) * RenderData.MaxVertices;

		RenderData.VertexBuffer = std::make_unique<BufferObject>(BufferObject::BufferType::VertexBuffer, VertexData);

		constexpr GLsizei VertexStride = sizeof(Vertex);

		RenderData.VertexArray->AttribPointer(0, 3, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex, Vertex::Position)); //All vertex attributes that will be passed to shader are set up here
		RenderData.VertexArray->AttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex, Vertex::Color));
		RenderData.VertexArray->AttribPointer(2, 3, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex, Vertex::Normal));
		RenderData.VertexArray->AttribPointer(3, 2, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex, Vertex::TextureCoordinates));
		RenderData.VertexArray->AttribPointer(4, 1, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex, Vertex::TextureUnit));
		RenderData.VertexArray->AttribPointer(5, 1, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex, Vertex::MaterialUnit));
		RenderData.VertexArray->AttribPointer(6, 4, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex, Vertex::ModelMatrix)); //col 0
		RenderData.VertexArray->AttribPointer(7, 4, GL_FLOAT, GL_FALSE, VertexStride, (void*)(offsetof(Vertex, Vertex::ModelMatrix) + (sizeof(GLfloat) * 4))); // col 1
		RenderData.VertexArray->AttribPointer(8, 4, GL_FLOAT, GL_FALSE, VertexStride, (void*)(offsetof(Vertex, Vertex::ModelMatrix) + (sizeof(GLfloat) * 8))); // col 2
		RenderData.VertexArray->AttribPointer(9, 4, GL_FLOAT, GL_FALSE, VertexStride, (void*)(offsetof(Vertex, Vertex::ModelMatrix) + (sizeof(GLfloat) * 12))); // col 3

		RenderData.CircleBuffer = new CircleVertex[RenderData.MaxVertices];
		RenderData.CircleBufferIndex = RenderData.CircleBuffer;

		std::map<GLenum, const char*> CircleShaderList =
		{
			{GL_VERTEX_SHADER,  "Engine/src/resources/shaders/Defaults/circleVertex.glsl"},
			{GL_FRAGMENT_SHADER, "Engine/src/resources/shaders/Defaults/circleFragment.glsl"}
		};

		RenderData.CircleShader = std::make_unique<Shader>(CircleShaderList);

		BufferData CircleVertexData;
		CircleVertexData.DrawType = GL_DYNAMIC_DRAW;
		CircleVertexData.data = nullptr;
		CircleVertexData.VertexSize = sizeof(CircleVertex) * RenderData.MaxVertices;

		RenderData.CircleVertexArray = std::make_unique<VertexArrayObject>();
		RenderData.CircleVertexArray->Create();
		RenderData.CircleVertexArray->Bind();

		RenderData.CircleVertexBuffer = std::make_unique<BufferObject>(BufferObject::BufferType::VertexBuffer, CircleVertexData);

		constexpr GLsizei CircleVertexStride = sizeof(CircleVertex);

		RenderData.CircleVertexArray->AttribPointer(0, 3, GL_FLOAT, GL_FALSE, CircleVertexStride, (void*)offsetof(CircleVertex, CircleVertex::Position));
		RenderData.CircleVertexArray->AttribPointer(1, 4, GL_FLOAT, GL_FALSE, CircleVertexStride, (void*)offsetof(CircleVertex, CircleVertex::Color));
		RenderData.CircleVertexArray->AttribPointer(2, 1, GL_FLOAT, GL_FALSE, CircleVertexStride, (void*)offsetof(CircleVertex, CircleVertex::Thickness));
		RenderData.CircleVertexArray->AttribPointer(3, 1, GL_FLOAT, GL_FALSE, CircleVertexStride, (void*)offsetof(CircleVertex, CircleVertex::MaterialUnit));
		RenderData.CircleVertexArray->AttribPointer(4, 4, GL_FLOAT, GL_FALSE, CircleVertexStride, (void*)offsetof(CircleVertex, CircleVertex::ModelMatrix));
		RenderData.CircleVertexArray->AttribPointer(5, 4, GL_FLOAT, GL_FALSE, CircleVertexStride, (void*)(offsetof(CircleVertex, CircleVertex::ModelMatrix) + (sizeof(GLfloat) * 4)));
		RenderData.CircleVertexArray->AttribPointer(6, 4, GL_FLOAT, GL_FALSE, CircleVertexStride, (void*)(offsetof(CircleVertex, CircleVertex::ModelMatrix) + (sizeof(GLfloat) * 8)));
		RenderData.CircleVertexArray->AttribPointer(7, 4, GL_FLOAT, GL_FALSE, CircleVertexStride, (void*)(offsetof(CircleVertex, CircleVertex::ModelMatrix) + (sizeof(GLfloat) * 12)));

		uint32_t offset = 0;
		for (int i = 0; i < RenderData.MaxIndicies; i += 6) //Init of indicies ready to be used for rendering
		{
			RenderData.Indicies[i] = 0 + offset;
			RenderData.Indicies[i + 1] = 1 + offset;
			RenderData.Indicies[i + 2] = 2 + offset;
			RenderData.Indicies[i + 3] = 2 + offset;
			RenderData.Indicies[i + 4] = 3 + offset;
			RenderData.Indicies[i + 5] = 0 + offset;
			offset += 4;

		}
		BufferData IndexData;
		IndexData.data = RenderData.Indicies;
		IndexData.DrawType = GL_STATIC_DRAW;
		IndexData.VertexSize = sizeof(uint32_t) * RenderData.MaxIndicies;

		RenderData.IndexBuffer = std::make_unique<BufferObject>(BufferObject::BufferType::IndexBuffer, IndexData);

		BufferData UniformData;
		UniformData.data = nullptr;
		UniformData.DrawType = GL_DYNAMIC_DRAW;
		UniformData.VertexSize = RenderData.MaterialStride * RenderData.MaxMaterialSlots;

		RenderData.UniformBuffer = std::make_unique<BufferObject>(BufferObject::BufferType::UniformBuffer, UniformData);

		DynamicData DynamicUniformRange;
		DynamicUniformRange.Offset = 0;
		DynamicUniformRange.index = 0;
		DynamicUniformRange.VertexSize = RenderData.MaterialStride * RenderData.MaxMaterialSlots;

		RenderData.UniformBuffer->BindRange(DynamicUniformRange);

		DynamicData DefaultMatData;
		DefaultMatData.data = &DefaultMaterial;
		DefaultMatData.Offset = 0;
		DefaultMatData.VertexSize = RenderData.MaterialStride;

		RenderData.UniformBuffer->PushData(DefaultMatData); //Pushes default material into first material slot

		BufferData LightUniformData;
		LightUniformData.data = nullptr;
		LightUniformData.DrawType = GL_DYNAMIC_DRAW;
		LightUniformData.VertexSize = RenderData.LightStride * RenderData.MaxLightSources;

		RenderData.LightUniformBuffer = std::make_unique<BufferObject>(BufferObject::BufferType::UniformBuffer, LightUniformData);

		DynamicData LightUniformRange;
		LightUniformRange.index = 1;
		LightUniformRange.Offset = 0;
		LightUniformRange.VertexSize = RenderData.LightStride * RenderData.MaxLightSources;

		RenderData.LightUniformBuffer->BindRange(LightUniformRange);

		RenderData.AllMaterials[0] = DefaultMaterial;
		RenderData.CurrentMaterialSlot += 1;

		glCreateTextures(GL_TEXTURE_2D, 1, &RenderData.DefaultTexture);
		glBindTexture(GL_TEXTURE_2D, RenderData.DefaultTexture);
		uint32_t colour = 0xffffffff;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &colour);
		RenderData.TextureSlots[0] = RenderData.DefaultTexture;

	}
	void Renderer2D::BeginScene(OrthoGraphicCamera& Camera)
	{
		RenderData.BufferIndex = RenderData.Buffer;
		RenderData.CircleBufferIndex = RenderData.CircleBuffer;

		RenderData.Camera = Camera;

		RenderData.DefaultShader->Use();
		RenderData.DefaultShader->setUniformMat4("view", Camera.GetView());
		RenderData.DefaultShader->setUniformMat4("projection", Camera.GetProjection());
		RenderData.DefaultShader->SetUniformFloat3("CameraPosition", RenderData.Camera.GetPosition().x,
			RenderData.Camera.GetPosition().y, RenderData.Camera.GetPosition().z);

		RenderData.CircleShader->Use();
		RenderData.CircleShader->setUniformMat4("view", Camera.GetView());
		RenderData.CircleShader->setUniformMat4("projection", Camera.GetProjection());
		RenderData.CircleShader->SetUniformFloat3("CameraPosition", RenderData.Camera.GetPosition().x, 
			RenderData.Camera.GetPosition().y, RenderData.Camera.GetPosition().z);


	}
	void Renderer2D::BeginScene()
	{
		RenderData.BufferIndex = RenderData.Buffer;
		RenderData.CircleBufferIndex = RenderData.CircleBuffer;

	}

	void Renderer2D::PushQuad(const glm::vec3& Position, const glm::vec2& size, float Rotation, uint32_t ID)
	{

		if (RenderData.IndexCount >= RenderData.MaxIndicies || RenderData.CurrentTextureSlot > (uint32_t)RenderData.MaxTextureSlots - 1) {
			FlushScene();
		}

		if (RenderData.CurrentTextureSlot > (uint32_t)RenderData.MaxTextureSlots - 1)
			RenderData.CurrentTextureSlot = 1;

		float TextureIndex = 0.0f;


		for (int i = 1; i < (int)RenderData.CurrentTextureSlot; i++)
		{
			if (RenderData.TextureSlots[i] == ID)
			{
				TextureIndex = (float)i;
				break;
			}
		}

		if (TextureIndex == 0.0f)
		{
			TextureIndex = (float)RenderData.CurrentTextureSlot;
			RenderData.TextureSlots[RenderData.CurrentTextureSlot] = ID;
			RenderData.CurrentTextureSlot++;

		}

		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, Position)
			* glm::rotate(ModelMatrix, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(ModelMatrix, glm::vec3(size, 1.0f));

		RenderData.BufferIndex = CreateQuad(RenderData.BufferIndex, TextureIndex, 0.0f, ModelMatrix);

		RenderData.IndexCount += 6;
	}
	void Renderer2D::PushQuad(const glm::vec3& Position, const glm::vec2& size, float Rotation, const glm::vec4& color)
	{
		if (RenderData.IndexCount >= RenderData.MaxIndicies) {
			FlushScene();
		}

		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, Position)
			* glm::rotate(ModelMatrix, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(ModelMatrix, glm::vec3(size, 1.0f));

		RenderData.BufferIndex = CreateQuad(RenderData.BufferIndex, color, 0.0f, ModelMatrix);

		RenderData.IndexCount += 6;
	}
	void Renderer2D::PushSprite(const glm::vec3& Position, const glm::vec2& size, float Rotation, uint32_t ID, SpriteSheet sheet)
	{
		if (RenderData.IndexCount >= RenderData.MaxIndicies || RenderData.CurrentTextureSlot > (uint32_t)RenderData.MaxTextureSlots - 1) {
			FlushScene();
		}

		if (RenderData.CurrentTextureSlot > (uint32_t)RenderData.MaxTextureSlots - 1)
			RenderData.CurrentTextureSlot = 1;

		float TextureIndex = 0.0f;


		for (int i = 1; i < (int)RenderData.CurrentTextureSlot; ++i)
		{
			if (RenderData.TextureSlots[i] == ID)
			{
				TextureIndex = (float)i;
				break;
			}
		}

		if (TextureIndex == 0.0f)
		{
			RenderData.TextureSlots[RenderData.CurrentTextureSlot] = ID;
			TextureIndex = (float)RenderData.CurrentTextureSlot;

			RenderData.CurrentTextureSlot++;
		}

		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, Position)
			* glm::rotate(ModelMatrix, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(ModelMatrix, glm::vec3(size, 1.0f));

		RenderData.BufferIndex = CreateSprite(RenderData.BufferIndex, sheet, TextureIndex, 0.0f, ModelMatrix);

		RenderData.IndexCount += 6;

	}
	void Renderer2D::PushQuad(const glm::vec3& Position, const glm::vec2& size, float Rotation, uint32_t ID, Material& material)
	{
		if (RenderData.IndexCount >= RenderData.MaxIndicies || RenderData.CurrentTextureSlot > (uint32_t)RenderData.MaxTextureSlots - 1
			|| RenderData.CurrentMaterialSlot > RenderData.MaxMaterialSlots) {
			FlushScene();
		}

		if (RenderData.CurrentTextureSlot > (uint32_t)RenderData.MaxTextureSlots - 1)
			RenderData.CurrentTextureSlot = 1;

		if (RenderData.CurrentMaterialSlot > RenderData.MaxMaterialSlots)
			RenderData.CurrentMaterialSlot = 1;




		float TextureIndex = 0.0f;


		for (int i = 1; i < (int)RenderData.CurrentTextureSlot; i++)
		{
			if (RenderData.TextureSlots[i] == ID)
			{
				TextureIndex = (float)i;
				break;
			}
		}

		if (TextureIndex == 0.0f)
		{
			TextureIndex = (float)RenderData.CurrentTextureSlot;
			RenderData.TextureSlots[RenderData.CurrentTextureSlot] = ID;
			RenderData.CurrentTextureSlot++;

		}


		float MaterialIndex = 0.0f;

		for (int i = 1; i < (int)RenderData.CurrentMaterialSlot; i++)
		{
			if (RenderData.AllMaterials[i] == material)
			{
				MaterialIndex = (float)i;
				break;
			}

		}

		if (MaterialIndex == 0.0f)
		{
			MaterialIndex = (float)RenderData.CurrentMaterialSlot;
			RenderData.AllMaterials[RenderData.CurrentMaterialSlot] = material;
			RenderData.CurrentMaterialSlot++;
		}

		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, Position)
			* glm::rotate(ModelMatrix, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(ModelMatrix, glm::vec3(size, 1.0f));

		RenderData.BufferIndex = CreateQuad(RenderData.BufferIndex, TextureIndex, MaterialIndex, ModelMatrix);

		RenderData.IndexCount += 6;
	}
	void Renderer2D::PushQuad(const glm::vec3& Position, const glm::vec2& size, float Rotation, const glm::vec4& color, Material& material)
	{

		if (RenderData.IndexCount >= RenderData.MaxIndicies || RenderData.CurrentMaterialSlot > RenderData.MaxMaterialSlots) {
			FlushScene();
		}

		if (RenderData.CurrentMaterialSlot > RenderData.MaxMaterialSlots)
			RenderData.CurrentMaterialSlot = 1;




		float MaterialIndex = 0.0f;

		for (int i = 1; i < (int)RenderData.CurrentMaterialSlot; i++)
		{
			if (RenderData.AllMaterials[i] == material)
			{
				MaterialIndex = (float)i;
				break;
			}

		}

		if (MaterialIndex == 0.0f)
		{
			MaterialIndex = (float)RenderData.CurrentMaterialSlot;
			RenderData.AllMaterials[RenderData.CurrentMaterialSlot] = material;
			RenderData.CurrentMaterialSlot++;
		}

		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, Position)
			* glm::rotate(ModelMatrix, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(ModelMatrix, glm::vec3(size, 1.0f));

		RenderData.BufferIndex = CreateQuad(RenderData.BufferIndex, color, MaterialIndex, ModelMatrix);

		RenderData.IndexCount += 6;

	}
	void Renderer2D::PushSprite(const glm::vec3& Position, const glm::vec2& size, float Rotation, uint32_t ID, SpriteSheet sheet, Material& material)
	{
		if (RenderData.IndexCount >= RenderData.MaxIndicies || RenderData.CurrentTextureSlot > (uint32_t)RenderData.MaxTextureSlots - 1
			|| RenderData.CurrentMaterialSlot > RenderData.MaxMaterialSlots) {
			FlushScene();
		}

		if (RenderData.CurrentTextureSlot > (uint32_t)RenderData.MaxTextureSlots - 1)
			RenderData.CurrentTextureSlot = 1;

		if (RenderData.CurrentMaterialSlot > RenderData.MaxMaterialSlots)
			RenderData.CurrentMaterialSlot = 1;


		float TextureIndex = 0.0f;


		for (int i = 0; i < (int)RenderData.CurrentTextureSlot; ++i)
		{
			if (RenderData.TextureSlots[i] == ID)
			{
				TextureIndex = (float)i;
				break;
			}
		}

		if (TextureIndex == 0.0f)
		{
			RenderData.TextureSlots[RenderData.CurrentTextureSlot] = ID;
			TextureIndex = (float)RenderData.CurrentTextureSlot;

			RenderData.CurrentTextureSlot++;
		}

		float MaterialIndex = 0.0f;

		for (int i = 1; i < (int)RenderData.CurrentMaterialSlot; i++)
		{
			if (RenderData.AllMaterials[i] == material)
			{
				MaterialIndex = (float)i;
				break;
			}

		}

		if (MaterialIndex == 0.0f)
		{
			MaterialIndex = (float)RenderData.CurrentMaterialSlot;
			RenderData.AllMaterials[RenderData.CurrentMaterialSlot] = material;
			RenderData.CurrentMaterialSlot++;
		}

		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, Position)
			* glm::rotate(ModelMatrix, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(ModelMatrix, glm::vec3(size, 1.0f));

		RenderData.BufferIndex = CreateSprite(RenderData.BufferIndex, sheet, TextureIndex, MaterialIndex, ModelMatrix);

		RenderData.IndexCount += 6;
	}
	void Renderer2D::PushCircle(const glm::vec3& Position, const glm::vec2& size, float Rotation, float Thickness, const glm::vec4& color)
	{
		if (RenderData.CircleIndexCount >= RenderData.MaxVertices ) {
			FlushScene();
		}

		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, Position)
			* glm::rotate(ModelMatrix, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(ModelMatrix, glm::vec3(size, 1.0f));


		RenderData.CircleBufferIndex = CreateCircle(RenderData.CircleBufferIndex, Thickness, color, 0.0f, ModelMatrix);
																									 
		RenderData.CircleIndexCount += 6;
	}
	void Renderer2D::PushCircle(const glm::vec3& Position, const glm::vec2& size, float Rotation, float Thickness, const glm::vec4& color, Material& material)
	{
		if (RenderData.CircleIndexCount >= RenderData.MaxVertices 
			|| RenderData.CurrentMaterialSlot > RenderData.MaxMaterialSlots) {
			FlushScene();
		}

		if (RenderData.CurrentMaterialSlot > RenderData.MaxMaterialSlots)
			RenderData.CurrentMaterialSlot = 1;

		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, Position)
			* glm::rotate(ModelMatrix, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(ModelMatrix, glm::vec3(size, 1.0f));


		float MaterialIndex = 0.0f;

		for (int i = 1; i < (int)RenderData.CurrentMaterialSlot; i++)
		{
			if (RenderData.AllMaterials[i] == material)
			{
				MaterialIndex = (float)i;
				break;
			}

		}

		if (MaterialIndex == 0.0f)
		{
			MaterialIndex = (float)RenderData.CurrentMaterialSlot;
			RenderData.AllMaterials[RenderData.CurrentMaterialSlot] = material;
			RenderData.CurrentMaterialSlot++;
		}

		RenderData.CircleBufferIndex = CreateCircle(RenderData.CircleBufferIndex, Thickness, color, MaterialIndex, ModelMatrix);

		RenderData.CircleIndexCount += 6;
	}
	void Renderer2D::PushLight(const LightSource& light)
	{
		if (RenderData.CurrentLightSlot >= RenderData.MaxLightSources)
		{
			FlushScene();
			RenderData.CurrentLightSlot = 0;
		}

		bool isFound = false;

		for (int i = 0; i < RenderData.CurrentLightSlot; i++)
		{
			if (RenderData.LightSources[i] == light)
			{
				isFound = true;
				if (RenderData.LightSources[i].Position != light.Position) //this ensures if the light source position/color changes it updates
					RenderData.LightSources[i].Position = light.Position;

				if (RenderData.LightSources[i].Color != light.Color)
					RenderData.LightSources[i].Color = light.Color;

				break;
			}
		}


		if (!isFound) //if not found creates a new light and addes it to the light slots array
		{
			RenderData.LightSources[RenderData.CurrentLightSlot] = light;
			RenderData.LightSources[RenderData.CurrentLightSlot].ID = (float)RenderData.CurrentLightSlot;
			RenderData.CurrentLightSlot += 1;
		}


	}
	void Renderer2D::DrawQuad()
	{
		RenderData.VertexArray->Bind();
		RenderData.DefaultShader->Use();

		RenderData.VertexBuffer->Bind();
		RenderData.IndexBuffer->Bind();

		for (int i = 0; i < (int)RenderData.CurrentTextureSlot; i++)
			glBindTextureUnit(i, RenderData.TextureSlots[i]);


		glDrawElements(GL_TRIANGLES, RenderData.IndexCount, GL_UNSIGNED_INT, nullptr);

		RenderData.IndexCount = 0;

	}
	void Renderer2D::DrawCircle() //checked off
	{
		RenderData.CircleShader->Use();
		RenderData.CircleVertexArray->Bind();

		RenderData.IndexBuffer->Bind();

		glDrawElements(GL_TRIANGLES, RenderData.CircleIndexCount, GL_UNSIGNED_INT, nullptr);

		RenderData.CircleIndexCount = 0;

	}
	void Renderer2D::EndScene() //checked off
	{


		RenderData.VertexArray->Bind();
		GLsizeiptr QuadSize = (uint8_t*)RenderData.BufferIndex - (uint8_t*)RenderData.Buffer;

		DynamicData Buffer;
		Buffer.data = RenderData.Buffer;
		Buffer.Offset = 0;
		Buffer.VertexSize = QuadSize;

		RenderData.VertexBuffer->PushData(Buffer);

		Buffer.data = RenderData.AllMaterials.data() + 1;
		Buffer.Offset = RenderData.MaterialStride;
		Buffer.VertexSize = RenderData.MaterialStride * (RenderData.MaxMaterialSlots - 1);


		RenderData.UniformBuffer->PushData(Buffer);

		Buffer.data = RenderData.LightSources.data();
		Buffer.Offset = 0;
		Buffer.VertexSize = RenderData.CurrentLightSlot * RenderData.LightStride;

		RenderData.LightUniformBuffer->PushData(Buffer);

		RenderData.DefaultShader->Use();
		RenderData.DefaultShader->SetUniformFloat("NumberOfLightSources", (float)RenderData.CurrentLightSlot);


		RenderData.CircleVertexArray->Bind();
		GLsizeiptr CircleSize = (uint8_t*)RenderData.CircleBufferIndex - (uint8_t*)RenderData.CircleBuffer;

		RenderData.CircleShader->Use();
		RenderData.CircleShader->SetUniformFloat("NumberOfLightSources", (float)RenderData.CurrentLightSlot);

		DynamicData CircleBufferData;
		CircleBufferData.data = RenderData.CircleBuffer;
		CircleBufferData.Offset = 0;
		CircleBufferData.VertexSize = CircleSize;

		RenderData.CircleVertexBuffer->PushData(CircleBufferData);

		DrawQuad();
		DrawCircle();


	}

	void Renderer2D::ShutDown()
	{
		glDeleteTextures(1, &RenderData.DefaultTexture);

		delete[] RenderData.Buffer;
		delete[] RenderData.CircleBuffer;
		
	}
	void Renderer2D::FlushScene()
	{
		EndScene();
		BeginScene();
	}
	void Renderer2D::Clear()
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	void Renderer2D::ClearColor(const glm::vec3& Color)
	{
		glClearColor(Color.x, Color.y, Color.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Renderer2D::ClearDepth(const glm::vec3& Color)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glClearColor(Color.x, Color.y, Color.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Renderer2D::SetUniformVec3(const glm::vec3& data, const char* name)
	{
		RenderData.DefaultShader->SetUniformFloat3(name, data.x, data.y, data.z);
	}

	

	void Raytracing2D::Init()
	{
		std::map<GLenum, const char*> ShaderList = {
			{GL_VERTEX_SHADER, "Engine/src/resources/shaders/Defaults/RayTracing/basicVertex.glsl"},
			{GL_FRAGMENT_SHADER, "Engine/src/resources/shaders/Defaults/RayTracing/basicFragment.glsl"},

		};

		RaytracingData.DefaultShader = std::make_unique<Shader>(ShaderList);

		RaytracingData.CurrentTextureSlot = 1;

		RaytracingData.Buffer = new Vertex[RaytracingData.MaxVertices];
		RaytracingData.BufferIndex = RaytracingData.Buffer;

		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &RaytracingData.MaxTextureSlots); //Queries the drivers for the maximum texture slot of the gpu
		std::string msg = std::to_string(RaytracingData.MaxTextureSlots) + " Texture Slots";
		LOG(msg);

		RaytracingData.MaxTextureSlots = 32;

		RaytracingData.AllMaterials.resize(RaytracingData.MaxMaterialSlots);

		Material DefaultMaterial;
		DefaultMaterial.ambient = glm::vec3(0.2, 0.2, 0.2);
		DefaultMaterial.diffuse = glm::vec3(1.0, 1.0, 1.0);
		DefaultMaterial.specular = glm::vec3(0.5, 0.5, 0.5);
		DefaultMaterial.shininess = 32.0;


		for (int i = 0; i < (int)RaytracingData.MaxMaterialSlots - 1; i++)
			RaytracingData.AllMaterials[i] = DefaultMaterial;

		int* samplers = new int[RaytracingData.MaxTextureSlots];
		for (int i = 0; i < (int)RaytracingData.MaxTextureSlots; i++)
			samplers[i] = i;




		RaytracingData.DefaultShader->Use();
		RaytracingData.DefaultShader->SetUniformIntV("samplerTextures", samplers, RaytracingData.MaxTextureSlots);

		RaytracingData.TextureSlots.resize(RaytracingData.MaxTextureSlots);

		memset(RaytracingData.TextureSlots.data(), 0, RaytracingData.MaxTextureSlots);

		RaytracingData.VertexArray = std::make_unique<VertexArrayObject>();
		RaytracingData.VertexArray->Create();

		BufferData VertexData;
		VertexData.DrawType = GL_DYNAMIC_DRAW;
		VertexData.data = nullptr;
		VertexData.VertexSize = sizeof(Vertex) * RaytracingData.MaxVertices;

		RaytracingData.VertexBuffer = std::make_unique<BufferObject>(BufferObject::BufferType::VertexBuffer, VertexData);

		constexpr GLsizei VertexStride = sizeof(Vertex);

		RaytracingData.VertexArray->AttribPointer(0, 3, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex, Vertex::Position)); //All vertex attributes that will be passed to shader are set up here
		RaytracingData.VertexArray->AttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex, Vertex::Color));
		RaytracingData.VertexArray->AttribPointer(2, 3, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex, Vertex::Normal));
		RaytracingData.VertexArray->AttribPointer(3, 2, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex, Vertex::TextureCoordinates));
		RaytracingData.VertexArray->AttribPointer(4, 1, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex, Vertex::TextureUnit));
		RaytracingData.VertexArray->AttribPointer(5, 1, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex, Vertex::MaterialUnit));
		RaytracingData.VertexArray->AttribPointer(6, 4, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex, Vertex::ModelMatrix)); //col 0
		RaytracingData.VertexArray->AttribPointer(7, 4, GL_FLOAT, GL_FALSE, VertexStride, (void*)(offsetof(Vertex, Vertex::ModelMatrix) + (sizeof(GLfloat) * 4))); // col 1
		RaytracingData.VertexArray->AttribPointer(8, 4, GL_FLOAT, GL_FALSE, VertexStride, (void*)(offsetof(Vertex, Vertex::ModelMatrix) + (sizeof(GLfloat) * 8))); // col 2
		RaytracingData.VertexArray->AttribPointer(9, 4, GL_FLOAT, GL_FALSE, VertexStride, (void*)(offsetof(Vertex, Vertex::ModelMatrix) + (sizeof(GLfloat) * 12))); // col 3

		RaytracingData.CircleBuffer = new CircleVertex[RaytracingData.MaxVertices];
		RaytracingData.CircleBufferIndex = RaytracingData.CircleBuffer;

		std::map<GLenum, const char*> CircleShaderList =
		{
			{GL_VERTEX_SHADER,  "Engine/src/resources/shaders/Defaults/RayTracing/circleVertex.glsl"},
			{GL_FRAGMENT_SHADER, "Engine/src/resources/shaders/Defaults/RayTracing/circleFragment.glsl"}
		};

		RaytracingData.CircleShader = std::make_unique<Shader>(CircleShaderList);

		BufferData CircleVertexData;
		CircleVertexData.DrawType = GL_DYNAMIC_DRAW;
		CircleVertexData.data = nullptr;
		CircleVertexData.VertexSize = sizeof(CircleVertex) * RaytracingData.MaxVertices;

		RaytracingData.CircleVertexArray = std::make_unique<VertexArrayObject>();
		RaytracingData.CircleVertexArray->Create();
		RaytracingData.CircleVertexArray->Bind();

		RaytracingData.CircleVertexBuffer = std::make_unique<BufferObject>(BufferObject::BufferType::VertexBuffer, CircleVertexData);

		constexpr GLsizei CircleVertexStride = sizeof(CircleVertex);

		RaytracingData.CircleVertexArray->AttribPointer(0, 3, GL_FLOAT, GL_FALSE, CircleVertexStride, (void*)offsetof(CircleVertex, CircleVertex::Position));
		RaytracingData.CircleVertexArray->AttribPointer(1, 4, GL_FLOAT, GL_FALSE, CircleVertexStride, (void*)offsetof(CircleVertex, CircleVertex::Color));
		RaytracingData.CircleVertexArray->AttribPointer(2, 1, GL_FLOAT, GL_FALSE, CircleVertexStride, (void*)offsetof(CircleVertex, CircleVertex::Thickness));
		RaytracingData.CircleVertexArray->AttribPointer(3, 1, GL_FLOAT, GL_FALSE, CircleVertexStride, (void*)offsetof(CircleVertex, CircleVertex::MaterialUnit));
		RaytracingData.CircleVertexArray->AttribPointer(4, 4, GL_FLOAT, GL_FALSE, CircleVertexStride, (void*)offsetof(CircleVertex, CircleVertex::ModelMatrix));
		RaytracingData.CircleVertexArray->AttribPointer(5, 4, GL_FLOAT, GL_FALSE, CircleVertexStride, (void*)(offsetof(CircleVertex, CircleVertex::ModelMatrix) + (sizeof(GLfloat) * 4)));
		RaytracingData.CircleVertexArray->AttribPointer(6, 4, GL_FLOAT, GL_FALSE, CircleVertexStride, (void*)(offsetof(CircleVertex, CircleVertex::ModelMatrix) + (sizeof(GLfloat) * 8)));
		RaytracingData.CircleVertexArray->AttribPointer(7, 4, GL_FLOAT, GL_FALSE, CircleVertexStride, (void*)(offsetof(CircleVertex, CircleVertex::ModelMatrix) + (sizeof(GLfloat) * 12)));

		uint32_t offset = 0;
		for (int i = 0; i < RaytracingData.MaxIndicies; i += 6) //Init of indicies ready to be used for rendering
		{
			RaytracingData.Indicies[i] = 0 + offset;
			RaytracingData.Indicies[i + 1] = 1 + offset;
			RaytracingData.Indicies[i + 2] = 2 + offset;
			RaytracingData.Indicies[i + 3] = 2 + offset;
			RaytracingData.Indicies[i + 4] = 3 + offset;
			RaytracingData.Indicies[i + 5] = 0 + offset;
			offset += 4;

		}
		BufferData IndexData;
		IndexData.data = RaytracingData.Indicies;
		IndexData.DrawType = GL_STATIC_DRAW;
		IndexData.VertexSize = sizeof(uint32_t) * RaytracingData.MaxIndicies;

		RaytracingData.IndexBuffer = std::make_unique<BufferObject>(BufferObject::BufferType::IndexBuffer, IndexData);

		BufferData UniformData;
		UniformData.data = nullptr;
		UniformData.DrawType = GL_DYNAMIC_DRAW;
		UniformData.VertexSize = RaytracingData.MaterialStride * RaytracingData.MaxMaterialSlots;

		RaytracingData.UniformBuffer = std::make_unique<BufferObject>(BufferObject::BufferType::UniformBuffer, UniformData);

		DynamicData DynamicUniformRange;
		DynamicUniformRange.Offset = 0;
		DynamicUniformRange.index = 0;
		DynamicUniformRange.VertexSize = RaytracingData.MaterialStride * RaytracingData.MaxMaterialSlots;

		RaytracingData.UniformBuffer->BindRange(DynamicUniformRange);

		DynamicData DefaultMatData;
		DefaultMatData.data = &DefaultMaterial;
		DefaultMatData.Offset = 0;
		DefaultMatData.VertexSize = RaytracingData.MaterialStride;

		RaytracingData.UniformBuffer->PushData(DefaultMatData); //Pushes default material into first material slot

		BufferData LightUniformData;
		LightUniformData.data = nullptr;
		LightUniformData.DrawType = GL_DYNAMIC_DRAW;
		LightUniformData.VertexSize = RaytracingData.LightStride * RaytracingData.MaxLightSources;

		RaytracingData.LightUniformBuffer = std::make_unique<BufferObject>(BufferObject::BufferType::UniformBuffer, LightUniformData);

		DynamicData LightUniformRange;
		LightUniformRange.index = 1;
		LightUniformRange.Offset = 0;
		LightUniformRange.VertexSize = RaytracingData.LightStride * RaytracingData.MaxLightSources;

		RaytracingData.LightUniformBuffer->BindRange(LightUniformRange);

		RaytracingData.AllMaterials[0] = DefaultMaterial;
		RaytracingData.CurrentMaterialSlot += 1;

		glCreateTextures(GL_TEXTURE_2D, 1, &RaytracingData.DefaultTexture);
		glBindTexture(GL_TEXTURE_2D, RaytracingData.DefaultTexture);
		uint32_t colour = 0xffffffff;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &colour);
		RaytracingData.TextureSlots[0] = RaytracingData.DefaultTexture;

		GLint maxSSBOSize;
		glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxSSBOSize);
		msg = "Available shader storage block size: " + std::to_string(maxSSBOSize) + " bytes";
		LOG(msg);

		BufferData ShaderBufferData;
		ShaderBufferData.data = nullptr;
		ShaderBufferData.DrawType = GL_DYNAMIC_COPY;
		GLsizei AvailableSize = (maxSSBOSize / (sizeof(GLfloat) * 4)) / 5; //should use a 5th of available memory

		ShaderBufferData.VertexSize = sizeof(GLfloat) * 4 * AvailableSize;

		RaytracingData.ShaderStorage = std::make_unique<BufferObject>(
			BufferObject::BufferType::ShaderStorageBuffer, ShaderBufferData);
		RaytracingData.ShaderStorage->BindBase(2);

	}
	void Raytracing2D::BeginScene(OrthoGraphicCamera& Camera)
	{
		RaytracingData.BufferIndex = RaytracingData.Buffer;
		RaytracingData.CircleBufferIndex = RaytracingData.CircleBuffer;

		RaytracingData.Camera = Camera;

		RaytracingData.DefaultShader->Use();
		RaytracingData.DefaultShader->setUniformMat4("view", Camera.GetView());
		RaytracingData.DefaultShader->setUniformMat4("projection", Camera.GetProjection());

		RaytracingData.CircleShader->Use();
		RaytracingData.CircleShader->setUniformMat4("view", Camera.GetView());
		RaytracingData.CircleShader->setUniformMat4("projection", Camera.GetProjection());
		RaytracingData.CircleShader->SetUniformFloat3("CameraPosition", RaytracingData.Camera.GetPosition().x,
			RaytracingData.Camera.GetPosition().y, RaytracingData.Camera.GetPosition().z);


	}
	void Raytracing2D::BeginScene()
	{
		RaytracingData.BufferIndex = RaytracingData.Buffer;
		RaytracingData.CircleBufferIndex = RaytracingData.CircleBuffer;

	}

	void Raytracing2D::PushQuad(const glm::vec3& Position, const glm::vec2& size, float Rotation, uint32_t ID)
	{

		if (RaytracingData.IndexCount >= RaytracingData.MaxIndicies || RaytracingData.CurrentTextureSlot > (uint32_t)RaytracingData.MaxTextureSlots - 1) {
			FlushScene();
		}

		if (RaytracingData.CurrentTextureSlot > (uint32_t)RaytracingData.MaxTextureSlots - 1)
			RaytracingData.CurrentTextureSlot = 1;

		float TextureIndex = 0.0f;


		for (int i = 1; i < (int)RaytracingData.CurrentTextureSlot; i++)
		{
			if (RaytracingData.TextureSlots[i] == ID)
			{
				TextureIndex = (float)i;
				break;
			}
		}

		if (TextureIndex == 0.0f)
		{
			TextureIndex = (float)RaytracingData.CurrentTextureSlot;
			RaytracingData.TextureSlots[RaytracingData.CurrentTextureSlot] = ID;
			RaytracingData.CurrentTextureSlot++;

		}

		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, Position)
			* glm::rotate(ModelMatrix, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(ModelMatrix, glm::vec3(size, 1.0f));

		RaytracingData.BufferIndex = CreateQuad(RaytracingData.BufferIndex, TextureIndex, 0.0f, ModelMatrix);


		RaytracingData.IndexCount += 6;
		//RaytracingData.ObjectAttributes.push_back({ Position, 1.0f });
	}
	void Raytracing2D::PushQuad(const glm::vec3& Position, const glm::vec2& size, float Rotation, const glm::vec4& color)
	{
		if (RaytracingData.IndexCount >= RaytracingData.MaxIndicies) {
			FlushScene();
		}

		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, Position)
			* glm::rotate(ModelMatrix, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(ModelMatrix, glm::vec3(size, 1.0f));

		RaytracingData.BufferIndex = CreateQuad(RaytracingData.BufferIndex, color, 0.0f, ModelMatrix);

		RaytracingData.IndexCount += 6;
		//RaytracingData.ObjectAttributes.push_back({ Position, 0.0f });


	}
	void Raytracing2D::PushSprite(const glm::vec3& Position, const glm::vec2& size, float Rotation, uint32_t ID, SpriteSheet sheet)
	{
		if (RaytracingData.IndexCount >= RaytracingData.MaxIndicies || RaytracingData.CurrentTextureSlot > (uint32_t)RaytracingData.MaxTextureSlots - 1) {
			FlushScene();
		}

		if (RaytracingData.CurrentTextureSlot > (uint32_t)RaytracingData.MaxTextureSlots - 1)
			RaytracingData.CurrentTextureSlot = 1;




		float TextureIndex = 0.0f;


		for (int i = 1; i < (int)RaytracingData.CurrentTextureSlot; ++i)
		{
			if (RaytracingData.TextureSlots[i] == ID)
			{
				TextureIndex = (float)i;
				break;
			}
		}

		if (TextureIndex == 0.0f)
		{
			RaytracingData.TextureSlots[RaytracingData.CurrentTextureSlot] = ID;
			TextureIndex = (float)RaytracingData.CurrentTextureSlot;

			RaytracingData.CurrentTextureSlot++;
		}

		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, Position)
			* glm::rotate(ModelMatrix, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(ModelMatrix, glm::vec3(size, 1.0f));

		RaytracingData.BufferIndex = CreateSprite(RaytracingData.BufferIndex, sheet, TextureIndex, 0.0f, ModelMatrix);

		//RaytracingData.ObjectAttributes.push_back({ Position, 0.0f });
		RaytracingData.IndexCount += 6;

	}
	void Raytracing2D::PushQuad(const glm::vec3& Position, const glm::vec2& size, float Rotation, uint32_t ID, Material& material)
	{
		if (RaytracingData.IndexCount >= RaytracingData.MaxIndicies || RaytracingData.CurrentTextureSlot > (uint32_t)RaytracingData.MaxTextureSlots - 1
			|| RaytracingData.CurrentMaterialSlot > RaytracingData.MaxMaterialSlots) {
			FlushScene();
		}

		if (RaytracingData.CurrentTextureSlot > (uint32_t)RaytracingData.MaxTextureSlots - 1)
			RaytracingData.CurrentTextureSlot = 1;

		if (RaytracingData.CurrentMaterialSlot > RaytracingData.MaxMaterialSlots)
			RaytracingData.CurrentMaterialSlot = 1;




		float TextureIndex = 0.0f;


		for (int i = 1; i < (int)RaytracingData.CurrentTextureSlot; i++)
		{
			if (RaytracingData.TextureSlots[i] == ID)
			{
				TextureIndex = (float)i;
				break;
			}
		}

		if (TextureIndex == 0.0f)
		{
			TextureIndex = (float)RaytracingData.CurrentTextureSlot;
			RaytracingData.TextureSlots[RaytracingData.CurrentTextureSlot] = ID;
			RaytracingData.CurrentTextureSlot++;

		}


		float MaterialIndex = 0.0f;

		for (int i = 1; i < (int)RaytracingData.CurrentMaterialSlot; i++)
		{
			if (RaytracingData.AllMaterials[i] == material)
			{
				MaterialIndex = (float)i;
				break;
			}

		}

		if (MaterialIndex == 0.0f)
		{
			MaterialIndex = (float)RaytracingData.CurrentMaterialSlot;
			RaytracingData.AllMaterials[RaytracingData.CurrentMaterialSlot] = material;
			RaytracingData.CurrentMaterialSlot++;
		}

		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, Position)
			* glm::rotate(ModelMatrix, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(ModelMatrix, glm::vec3(size, 1.0f));

		RaytracingData.BufferIndex = CreateQuad(RaytracingData.BufferIndex, TextureIndex, MaterialIndex, ModelMatrix);

		//RaytracingData.ObjectAttributes.push_back({ Position, 0.0f });
		RaytracingData.IndexCount += 6;
	}
	void Raytracing2D::PushQuad(const glm::vec3& Position, const glm::vec2& size, float Rotation, const glm::vec4& color, Material& material)
	{

		if (RaytracingData.IndexCount >= RaytracingData.MaxIndicies || RaytracingData.CurrentMaterialSlot > RaytracingData.MaxMaterialSlots) {
			FlushScene();
		}

		if (RaytracingData.CurrentMaterialSlot > RaytracingData.MaxMaterialSlots)
			RaytracingData.CurrentMaterialSlot = 1;




		float MaterialIndex = 0.0f;

		for (int i = 1; i < (int)RaytracingData.CurrentMaterialSlot; i++)
		{
			if (RaytracingData.AllMaterials[i] == material)
			{
				MaterialIndex = (float)i;
				break;
			}

		}

		if (MaterialIndex == 0.0f)
		{
			MaterialIndex = (float)RaytracingData.CurrentMaterialSlot;
			RaytracingData.AllMaterials[RaytracingData.CurrentMaterialSlot] = material;
			RaytracingData.CurrentMaterialSlot++;
		}

		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, Position)
			* glm::rotate(ModelMatrix, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(ModelMatrix, glm::vec3(size, 1.0f));

		RaytracingData.BufferIndex = CreateQuad(RaytracingData.BufferIndex, color, MaterialIndex, ModelMatrix);
		//RaytracingData.ObjectAttributes.push_back({ Position, 0.0f });

		RaytracingData.IndexCount += 6;

	}
	void Raytracing2D::PushSprite(const glm::vec3& Position, const glm::vec2& size, float Rotation, uint32_t ID, SpriteSheet sheet, Material& material)
	{
		if (RaytracingData.IndexCount >= RaytracingData.MaxIndicies || RaytracingData.CurrentTextureSlot > (uint32_t)RaytracingData.MaxTextureSlots - 1
			|| RaytracingData.CurrentMaterialSlot > RaytracingData.MaxMaterialSlots) {
			FlushScene();
		}

		if (RaytracingData.CurrentTextureSlot > (uint32_t)RaytracingData.MaxTextureSlots - 1)
			RaytracingData.CurrentTextureSlot = 1;

		if (RaytracingData.CurrentMaterialSlot > RaytracingData.MaxMaterialSlots)
			RaytracingData.CurrentMaterialSlot = 1;


		float TextureIndex = 0.0f;


		for (int i = 0; i < (int)RaytracingData.CurrentTextureSlot; ++i)
		{
			if (RaytracingData.TextureSlots[i] == ID)
			{
				TextureIndex = (float)i;
				break;
			}
		}

		if (TextureIndex == 0.0f)
		{
			RaytracingData.TextureSlots[RaytracingData.CurrentTextureSlot] = ID;
			TextureIndex = (float)RaytracingData.CurrentTextureSlot;

			RaytracingData.CurrentTextureSlot++;
		}

		float MaterialIndex = 0.0f;

		for (int i = 1; i < (int)RaytracingData.CurrentMaterialSlot; i++)
		{
			if (RaytracingData.AllMaterials[i] == material)
			{
				MaterialIndex = (float)i;
				break;
			}

		}

		if (MaterialIndex == 0.0f)
		{
			MaterialIndex = (float)RaytracingData.CurrentMaterialSlot;
			RaytracingData.AllMaterials[RaytracingData.CurrentMaterialSlot] = material;
			RaytracingData.CurrentMaterialSlot++;
		}

		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, Position)
			* glm::rotate(ModelMatrix, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(ModelMatrix, glm::vec3(size, 1.0f));

		RaytracingData.BufferIndex = CreateSprite(RaytracingData.BufferIndex, sheet, TextureIndex, MaterialIndex, ModelMatrix);
		//RaytracingData.ObjectAttributes.push_back({ Position, 0.0f });
		RaytracingData.IndexCount += 6;
	}
	void Raytracing2D::PushCircle(const glm::vec3& Position, const glm::vec2& size, float Rotation, float Thickness, const glm::vec4& color)
	{
		if (RaytracingData.CircleIndexCount >= RaytracingData.MaxVertices) {
			FlushScene();
		}

		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, Position)
			* glm::rotate(ModelMatrix, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(ModelMatrix, glm::vec3(size, 1.0f));


		RaytracingData.CircleBufferIndex = CreateCircle(RaytracingData.CircleBufferIndex, Thickness, color, 0.0f, ModelMatrix);


		RaytracingData.ObjectAttributes.push_back({ Position.x, Position.y, size.y, 0.0 }); //First 2 are positions, 3rd is radius, and 4th is object type
		RaytracingData.CircleIndexCount += 6;
	}
	void Raytracing2D::PushCircle(const glm::vec3& Position, const glm::vec2& size, float Rotation, float Thickness, const glm::vec4& color, Material& material)
	{
		if (RaytracingData.CircleIndexCount >= RaytracingData.MaxVertices
			|| RaytracingData.CurrentMaterialSlot > RaytracingData.MaxMaterialSlots) {
			FlushScene();
		}

		if (RaytracingData.CurrentMaterialSlot > RaytracingData.MaxMaterialSlots)
			RaytracingData.CurrentMaterialSlot = 1;

		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, Position)
			* glm::rotate(ModelMatrix, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::scale(ModelMatrix, glm::vec3(size, 1.0f));


		float MaterialIndex = 0.0f;

		for (int i = 1; i < (int)RaytracingData.CurrentMaterialSlot; i++)
		{
			if (RaytracingData.AllMaterials[i] == material)
			{
				MaterialIndex = (float)i;
				break;
			}

		}

		if (MaterialIndex == 0.0f)
		{
			MaterialIndex = (float)RaytracingData.CurrentMaterialSlot;
			RaytracingData.AllMaterials[RaytracingData.CurrentMaterialSlot] = material;
			RaytracingData.CurrentMaterialSlot++;
		}

		RaytracingData.CircleBufferIndex = CreateCircle(RaytracingData.CircleBufferIndex, Thickness, color, MaterialIndex, ModelMatrix);


		RaytracingData.ObjectAttributes.push_back({ Position.x, Position.y, size.y, 0.0 }); //First 2 are positions, 3rd is radius, and 4th is object type
		RaytracingData.CircleIndexCount += 6;
	}
	void Raytracing2D::PushLight(const LightSource& light)
	{
		if (RaytracingData.CurrentLightSlot >= RaytracingData.MaxLightSources)
		{
			FlushScene();
			RaytracingData.CurrentLightSlot = 0;
		}

		bool isFound = false;

		for (int i = 0; i < RaytracingData.CurrentLightSlot; i++)
		{
			if (RaytracingData.LightSources[i] == light)
			{
				isFound = true;
				if (RaytracingData.LightSources[i].Position != light.Position) //this ensures if the light source position/color changes it updates
					RaytracingData.LightSources[i].Position = light.Position;

				if (RaytracingData.LightSources[i].Color != light.Color)
					RaytracingData.LightSources[i].Color = light.Color;

				break;
			}
		}


		if (!isFound) //if not found creates a new light and addes it to the light slots array
		{
			RaytracingData.LightSources[RaytracingData.CurrentLightSlot] = light;
			RaytracingData.LightSources[RaytracingData.CurrentLightSlot].ID = (float)RaytracingData.CurrentLightSlot;
			RaytracingData.CurrentLightSlot += 1;
		}


	}
	void Raytracing2D::DrawQuad()
	{
		RaytracingData.VertexArray->Bind();
		RaytracingData.DefaultShader->Use();

		RaytracingData.VertexBuffer->Bind();
		RaytracingData.IndexBuffer->Bind();

		for (int i = 0; i < (int)RaytracingData.CurrentTextureSlot; i++)
			glBindTextureUnit(i, RaytracingData.TextureSlots[i]);


		glDrawElements(GL_TRIANGLES, RaytracingData.IndexCount, GL_UNSIGNED_INT, nullptr);

		RaytracingData.IndexCount = 0;

	}
	void Raytracing2D::DrawCircle() //checked off
	{
		RaytracingData.CircleShader->Use();
		RaytracingData.CircleVertexArray->Bind();

		RaytracingData.IndexBuffer->Bind();

		glDrawElements(GL_TRIANGLES, RaytracingData.CircleIndexCount, GL_UNSIGNED_INT, nullptr);

		RaytracingData.CircleIndexCount = 0;

	}
	void Raytracing2D::EndScene() //checked off
	{


		RaytracingData.VertexArray->Bind();
		GLsizeiptr QuadSize = (uint8_t*)RaytracingData.BufferIndex - (uint8_t*)RaytracingData.Buffer;

		DynamicData Buffer;
		Buffer.data = RaytracingData.Buffer;
		Buffer.Offset = 0;
		Buffer.VertexSize = QuadSize;

		RaytracingData.VertexBuffer->PushData(Buffer);

		Buffer.data = RaytracingData.AllMaterials.data() + 1;
		Buffer.Offset = RaytracingData.MaterialStride;
		Buffer.VertexSize = RaytracingData.MaterialStride * (RaytracingData.MaxMaterialSlots - 1);


		RaytracingData.UniformBuffer->PushData(Buffer);

		Buffer.data = RaytracingData.LightSources.data();
		Buffer.Offset = 0;
		Buffer.VertexSize = RaytracingData.CurrentLightSlot * RaytracingData.LightStride;

		RaytracingData.LightUniformBuffer->PushData(Buffer);

		RaytracingData.DefaultShader->Use();
		RaytracingData.DefaultShader->SetUniformFloat("NumberOfLightSources", (float)RaytracingData.CurrentLightSlot);



		RaytracingData.CircleVertexArray->Bind();
		GLsizeiptr CircleSize = (uint8_t*)RaytracingData.CircleBufferIndex - (uint8_t*)RaytracingData.CircleBuffer;


		RaytracingData.CircleShader->Use();
		RaytracingData.CircleShader->SetUniformFloat("NumberOfLightSources", (float)RaytracingData.CurrentLightSlot);
		RaytracingData.CircleShader->SetUniformFloat("NumberOfObjects", (float)RaytracingData.ObjectAttributes.size() + 1.0f);

		DynamicData CircleBufferData;
		CircleBufferData.data = RaytracingData.CircleBuffer;
		CircleBufferData.Offset = 0;
		CircleBufferData.VertexSize = CircleSize;

		RaytracingData.CircleVertexBuffer->PushData(CircleBufferData);

		DynamicData ShaderData;
		ShaderData.data = RaytracingData.ObjectAttributes.data();
		ShaderData.VertexSize = RaytracingData.ObjectAttributes.size() * sizeof(GLfloat) * 4;
		ShaderData.Offset = 0;

		RaytracingData.ShaderStorage->PushData(ShaderData);
		RaytracingData.ObjectAttributes.clear();

		DrawQuad();
		DrawCircle();


	}

	void Raytracing2D::ShutDown()
	{
		glDeleteTextures(1, &RaytracingData.DefaultTexture);

		delete[] RaytracingData.Buffer;
		delete[] RaytracingData.CircleBuffer;

	}
	void Raytracing2D::FlushScene()
	{
		EndScene();
		BeginScene();
	}
	void Raytracing2D::Clear()
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	void Raytracing2D::ClearColor(const glm::vec3& Color)
	{
		glClearColor(Color.x, Color.y, Color.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Raytracing2D::ClearDepth(const glm::vec3& Color)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glClearColor(Color.x, Color.y, Color.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Raytracing2D::SetUniformVec3(const glm::vec3& data, const char* name)
	{
		RaytracingData.DefaultShader->SetUniformFloat3(name, data.x, data.y, data.z);
	}

}