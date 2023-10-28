#include <pch.h>

#include "Renderer2D.h"


namespace
{
	static Graphics::RendererData2D RenderData;
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
		LOG(std::to_string(RenderData.MaxTextureSlots));

		RenderData.MaxTextureSlots = 32;

		RenderData.AllMaterials.resize(RenderData.MaxMaterialSlots);

		Material DefaultMaterial;
		DefaultMaterial.ambient = glm::vec3(0.2, 0.2, 0.2);
		DefaultMaterial.diffuse = glm::vec3(0.7, 0.2, 0.4);
		DefaultMaterial.specular = glm::vec3(0.5, 0.5, 0.5);
		DefaultMaterial.shininess = 32.0;


		for (int i = 0; i < RenderData.MaxMaterialSlots - 1; i++)
			RenderData.AllMaterials[i] = DefaultMaterial;

		int* samplers = new int[RenderData.MaxTextureSlots];
		for (int i = 0; i < RenderData.MaxTextureSlots; i++)
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

		const GLsizei VertexStride = sizeof(Vertex);

		RenderData.VertexArray->AttribPointer(0, 3, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex,  Vertex::Position)); //All vertex attributes that will be passed to shader are set up here
		RenderData.VertexArray->AttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex,  Vertex::Color));
		RenderData.VertexArray->AttribPointer(2, 3, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex,  Vertex::Normal));
		RenderData.VertexArray->AttribPointer(3, 2, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex,  Vertex::TextureCoordinates));
		RenderData.VertexArray->AttribPointer(4, 1, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex,  Vertex::TextureUnit));
		RenderData.VertexArray->AttribPointer(5, 1, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex,  Vertex::MaterialUnit));
		RenderData.VertexArray->AttribPointer(6, 4, GL_FLOAT, GL_FALSE, VertexStride, (void*)offsetof(Vertex,  Vertex::ModelMatrix)); //col 0
		RenderData.VertexArray->AttribPointer(7, 4, GL_FLOAT, GL_FALSE, VertexStride, (void*)(offsetof(Vertex, Vertex::ModelMatrix) + (sizeof(GLfloat) * 4))); // col 1
		RenderData.VertexArray->AttribPointer(8, 4, GL_FLOAT, GL_FALSE, VertexStride, (void*)(offsetof(Vertex, Vertex::ModelMatrix) + (sizeof(GLfloat) * 8))); // col 2
		RenderData.VertexArray->AttribPointer(9, 4, GL_FLOAT, GL_FALSE, VertexStride, (void*)(offsetof(Vertex, Vertex::ModelMatrix) + (sizeof(GLfloat) * 12))); // col 3



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
		RenderData.Camera = Camera;

		RenderData.DefaultShader->Use();
		RenderData.DefaultShader->setUniformMat4("view", Camera.GetView());
		RenderData.DefaultShader->setUniformMat4("projection", Camera.GetProjection());

	}
	void Renderer2D::BeginScene()
	{
		RenderData.BufferIndex = RenderData.Buffer;
	}

	void Renderer2D::PushQuad(const glm::vec3& Position, float size, uint32_t ID, const glm::mat4& ModelMatrix)
	{

		if (RenderData.IndexCount >= RenderData.MaxIndicies) {
			FlushScene();
		}


		if (RenderData.CurrentTextureSlot > RenderData.MaxTextureSlots - 1)
			RenderData.CurrentTextureSlot = 1;

		float TextureIndex = 0.0f;


		for (int i = 1; i < RenderData.CurrentTextureSlot; i++)
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

		RenderData.BufferIndex = CreateQuad(RenderData.BufferIndex, Position, {size, size}, TextureIndex, 0.0f, ModelMatrix);


		RenderData.IndexCount += 6;
	}
	void Renderer2D::PushQuad(const glm::vec3& Position, float size, const glm::vec4& color, const glm::mat4& ModelMatrix)
	{
		if (RenderData.IndexCount >= RenderData.MaxIndicies) {
			FlushScene();
		}


		RenderData.BufferIndex = CreateQuad(RenderData.BufferIndex, Position, {size, size}, color, 0.0f, ModelMatrix);

		RenderData.IndexCount += 6;

	}
	void Renderer2D::PushSprite(const glm::vec3& Position, float size, uint32_t ID, SpriteSheet sheet, const glm::mat4& ModelMatrix)
	{
		if (RenderData.IndexCount >= RenderData.MaxIndicies) {
			FlushScene();
		}

		if (RenderData.CurrentTextureSlot > RenderData.MaxTextureSlots - 1)
		{
			FlushScene();
			RenderData.CurrentTextureSlot = 1;
		}



		float TextureIndex = 0.0f;


		for (int i = 1; i < RenderData.CurrentTextureSlot; ++i)
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

		RenderData.BufferIndex = CreateSprite(RenderData.BufferIndex, Position, {size, size}, sheet, TextureIndex, 0.0f, ModelMatrix);

		RenderData.IndexCount += 6;

	}
	void Renderer2D::PushQuad(const glm::vec3& Position, float size, uint32_t ID, Material& material, const glm::mat4& ModelMatrix)
	{
		if (RenderData.IndexCount >= RenderData.MaxIndicies) {
			FlushScene();
		}

		if (RenderData.CurrentTextureSlot > RenderData.MaxTextureSlots - 1)
		{
			FlushScene();
			RenderData.CurrentTextureSlot = 1;
		}


		if (RenderData.CurrentMaterialSlot > RenderData.MaxMaterialSlots)
		{
			FlushScene();
			RenderData.CurrentMaterialSlot = 1;
		}



		float TextureIndex = 0.0f;


		for (int i = 1; i < RenderData.CurrentTextureSlot; i++)
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

		for (int i = 1; i < RenderData.CurrentMaterialSlot; i++)
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

		RenderData.BufferIndex = CreateQuad(RenderData.BufferIndex, Position, {size, size}, TextureIndex, MaterialIndex, ModelMatrix);

		RenderData.IndexCount += 6;
	}
	void Renderer2D::PushQuad(const glm::vec3& Position, float size, const glm::vec4& color, Material& material, const glm::mat4& ModelMatrix)
	{

		if (RenderData.IndexCount >= RenderData.MaxIndicies) {
			FlushScene();
		}

		if (RenderData.CurrentMaterialSlot > RenderData.MaxMaterialSlots)
		{
			FlushScene();
			RenderData.CurrentMaterialSlot = 1;
		}



		float MaterialIndex = 0.0f;

		for (int i = 1; i < RenderData.CurrentMaterialSlot; i++)
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


		RenderData.BufferIndex = CreateQuad(RenderData.BufferIndex, Position, {size, size}, color, MaterialIndex, ModelMatrix);

		RenderData.IndexCount += 6;

	}
	void Renderer2D::PushSprite(const glm::vec3& Position, float size, uint32_t ID, SpriteSheet sheet, Material& material, const glm::mat4& ModelMatrix)
	{
		if (RenderData.IndexCount >= RenderData.MaxIndicies) {
			FlushScene();
		}


		if (RenderData.CurrentTextureSlot > RenderData.MaxTextureSlots - 1)
		{
			FlushScene();
			RenderData.CurrentTextureSlot = 1;
		}

		if (RenderData.CurrentMaterialSlot > RenderData.MaxMaterialSlots)
		{
			FlushScene();
			RenderData.CurrentMaterialSlot = 1;
		}

		float TextureIndex = 0.0f;


		for (int i = 0; i < RenderData.CurrentTextureSlot; ++i)
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

		for (int i = 1; i < RenderData.CurrentMaterialSlot; i++)
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


		RenderData.BufferIndex = CreateSprite(RenderData.BufferIndex, Position, {size, size}, sheet, TextureIndex, MaterialIndex, ModelMatrix);
		RenderData.IndexCount += 6;
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
	void Renderer2D::Draw()
	{
		
		RenderData.DefaultShader->Use();

		RenderData.VertexArray->Bind();
		RenderData.IndexBuffer->Bind();

		for (int i = 0; i < RenderData.CurrentTextureSlot; i++) {
			glBindTextureUnit(i, RenderData.TextureSlots[i]);
		}

		glDrawElements(GL_TRIANGLES, RenderData.IndexCount, GL_UNSIGNED_INT, nullptr);

		RenderData.IndexCount = 0;
		
	}
	void Renderer2D::EndScene()
	{
		GLsizeiptr size = (uint8_t*)RenderData.BufferIndex - (uint8_t*)RenderData.Buffer;
		DynamicData Buffer;
		Buffer.data = RenderData.Buffer;
		Buffer.Offset = 0;
		Buffer.VertexSize = size;

		RenderData.VertexBuffer->PushData(Buffer);

		Buffer.data = RenderData.AllMaterials.data() + 1;
		Buffer.Offset = RenderData.MaterialStride;
		Buffer.VertexSize = RenderData.MaterialStride * (RenderData.MaxMaterialSlots - 1);


		RenderData.UniformBuffer->PushData(Buffer);

		Buffer.data = RenderData.LightSources.data();
		Buffer.Offset = 0;
		Buffer.VertexSize = RenderData.CurrentLightSlot * RenderData.LightStride;

		RenderData.LightUniformBuffer->PushData(Buffer);

		RenderData.DefaultShader->SetUniformFloat("NumberOfLightSources", RenderData.CurrentLightSlot);

		Draw();
	}

	void Renderer2D::ShutDown()
	{
		glDeleteTextures(1, &RenderData.DefaultTexture);

		delete[] RenderData.Buffer;

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

}