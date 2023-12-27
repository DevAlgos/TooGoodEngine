#include <pch.h>

#include "Renderer2D.h"

#include <filesystem>

namespace
{
	static TGE::RendererData2D RenderData;
	static TGE::RaytracingData s_RaytracingData;

	static glm::vec3 TestOrigin(0.0f);


	uint32_t RGBAtoARGB(const glm::vec4& Color)
	{
		uint8_t r = static_cast<uint8_t>(Color.r * 255.0f);
		uint8_t g = static_cast<uint8_t>(Color.g * 255.0f);
		uint8_t b = static_cast<uint8_t>(Color.b * 255.0f);
		uint8_t a = static_cast<uint8_t>(Color.a * 255.0f);

		return static_cast<uint32_t>((a << 24) | (b << 16) | (g << 8) | r);
	}
}

namespace TGE
{
	void Renderer2D::Init()
	{
#pragma region InitTextures
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

#pragma endregion InitTextures

#pragma region InitUI
		RenderData.UITextureCount = 0;
		RenderData.UITextureSlots.resize(RenderData.MaxTextureSlots);

		memset(RenderData.UITextureSlots.data(), 0, RenderData.UITextureSlots.size());

		std::map<GLenum, std::string_view> UIShaderList;
		UIShaderList[GL_VERTEX_SHADER] = "../Resources/Shaders/UI/UIVertex.glsl";
		UIShaderList[GL_FRAGMENT_SHADER] = "../Resources/Shaders/UI/UIFragment.glsl";

		RenderData.UIShaders = Shader::Generate(UIShaderList);
		RenderData.UIShaders->Use();
		RenderData.UIShaders->SetUniformIntV("samplerTextures", samplers, RenderData.MaxTextureSlots);

		RenderData.UIVao = VertexArrayObject::Generate();
		RenderData.UIVao->Bind();

		BufferData UIBufferData;
		UIBufferData.data = nullptr;
		UIBufferData.DrawType = GL_DYNAMIC_DRAW;
		UIBufferData.VertexSize = sizeof(UIVertex) * RenderData.MaxUIVertices;

		RenderData.UIVbo = OpenGLBuffer::Generate(BufferType::VertexBuffer, UIBufferData);

		RenderData.UIBuffer = new UIVertex[RenderData.MaxUIVertices];
		RenderData.UIBufferIndex = RenderData.UIBuffer;

		RenderData.UIVao->AttachAttribPointers(
			{
				AttributeType::FLOAT_3, //Position
				AttributeType::FLOAT_4, //Color
				AttributeType::FLOAT_2, //Texture Coordinate
				AttributeType::FLOAT,   //Texture Unit
				AttributeType::MAT_4	//Model/Transform Matrix

			});

#pragma endregion InitUI

#pragma region QuadInit
		std::map<GLenum, std::string_view> ShaderList = {
			{GL_VERTEX_SHADER, "../Resources/shaders/Defaults/basicVertex.glsl"},
			{GL_FRAGMENT_SHADER, "../Resources/shaders/Defaults/basicFragment.glsl"},

		};

		RenderData.DefaultShader = Shader::Generate(ShaderList);


		RenderData.DefaultShader->Use();
		RenderData.DefaultShader->SetUniformIntV("samplerTextures", samplers, RenderData.MaxTextureSlots);

		RenderData.TextureSlots.resize(RenderData.MaxTextureSlots);

		memset(RenderData.TextureSlots.data(), 0, RenderData.MaxTextureSlots);

		RenderData.VertexArray = VertexArrayObject::Generate();

		BufferData VertexData;
		VertexData.DrawType = GL_DYNAMIC_DRAW;
		VertexData.data = nullptr;
		VertexData.VertexSize = sizeof(Vertex) * RenderData.MaxVertices;

		RenderData.VertexBuffer = OpenGLBuffer::Generate(BufferType::VertexBuffer, VertexData);

		RenderData.VertexArray->AttachAttribPointers(
			{
				AttributeType::FLOAT_3, // Position
				AttributeType::FLOAT_4, // Color
				AttributeType::FLOAT_3, // Normal
				AttributeType::FLOAT_2, // Texture Coordiantes
				AttributeType::FLOAT,	// Texture Unit
				AttributeType::FLOAT,	// Material Unit
				AttributeType::MAT_4	// Model/Transform Matrix
			});

#pragma endregion QuadInit

#pragma region CircleInit
		RenderData.CircleBuffer = new CircleVertex[RenderData.MaxVertices];
		RenderData.CircleBufferIndex = RenderData.CircleBuffer;

		std::map<GLenum, std::string_view> CircleShaderList =
		{
			{GL_VERTEX_SHADER,  "../Resources/shaders/Defaults/circleVertex.glsl"},
			{GL_FRAGMENT_SHADER, "../Resources/shaders/Defaults/circleFragment.glsl"}
		};

		RenderData.CircleShader = Shader::Generate(CircleShaderList);

		BufferData CircleVertexData;
		CircleVertexData.DrawType = GL_DYNAMIC_DRAW;
		CircleVertexData.data = nullptr;
		CircleVertexData.VertexSize = sizeof(CircleVertex) * RenderData.MaxVertices;

		RenderData.CircleVertexArray = VertexArrayObject::Generate();
		RenderData.CircleVertexArray->Bind();

		RenderData.CircleVertexBuffer = OpenGLBuffer::Generate(BufferType::VertexBuffer, CircleVertexData);

		RenderData.CircleVertexArray->AttachAttribPointers(
			{
				AttributeType::FLOAT_3, //Position
				AttributeType::FLOAT_4, //Color
				AttributeType::FLOAT,	//Thickness
				AttributeType::FLOAT,	//Material Unit
				AttributeType::MAT_4	//Model/Transform Matrix
			});

#pragma endregion CircleInit

#pragma region GeneralInit
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

		RenderData.IndexBuffer = OpenGLBuffer::Generate(BufferType::IndexBuffer, IndexData);

		BufferData UniformData;
		UniformData.data = nullptr;
		UniformData.DrawType = GL_DYNAMIC_DRAW;
		UniformData.VertexSize = RenderData.MaterialStride * RenderData.MaxMaterialSlots;

		RenderData.UniformBuffer = OpenGLBuffer::Generate(BufferType::UniformBuffer, UniformData);

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

		RenderData.LightUniformBuffer = OpenGLBuffer::Generate(BufferType::UniformBuffer, LightUniformData);

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
#pragma endregion GeneralInit

	}
	void Renderer2D::BeginScene(OrthoGraphicCamera& Camera)
	{
		RenderData.BufferIndex = RenderData.Buffer;
		RenderData.CircleBufferIndex = RenderData.CircleBuffer;
		RenderData.UIBufferIndex = RenderData.UIBuffer;

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

		RenderData.UIShaders->Use();
		RenderData.UIShaders->setUniformMat4("view", Camera.GetView());
		RenderData.UIShaders->setUniformMat4("projection", Camera.GetProjection());

	}
	void Renderer2D::BeginScene()
	{
		RenderData.BufferIndex = RenderData.Buffer;
		RenderData.CircleBufferIndex = RenderData.CircleBuffer;
		RenderData.UIBufferIndex = RenderData.UIBuffer;
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
	void Renderer2D::PushUIText(const std::string_view& Text, uint32_t Font, const glm::vec3& Position, const glm::vec2& size, float Rotation, const glm::vec4& color)
	{
		const uint32_t TextureUnit = RenderData.UIManager.GetFont(Font).CharacterSheet.Get();

		if (RenderData.UIIndexCount >= RenderData.MaxIndicies || RenderData.UITextureCount > (uint32_t)RenderData.MaxTextureSlots - 1) 
		{
			FlushScene();
		}

		if (RenderData.UITextureCount > (uint32_t)RenderData.MaxTextureSlots - 1)
			RenderData.CurrentTextureSlot = 0;

		float TextureIndex = -1.0f;

		for (int i = 0; i < (int)RenderData.UITextureCount; i++)
		{
			if (RenderData.UITextureSlots[i] == TextureUnit)
			{
				TextureIndex = (float)i;
				break;
			}
		}

		if (TextureIndex == -1.0f)
		{
			RenderData.UITextureSlots[RenderData.UITextureCount] = TextureUnit;
			TextureIndex = TextureUnit;
			RenderData.UITextureCount++;
		}

		glm::vec3 CurrentPosition = Position;
		 
		for (int i = 0; i < Text.size(); i++)
		{
			if (std::isspace(Text[i]))
			{
				CurrentPosition.x += 0.5f;
				continue;
			}

			const Character& character = RenderData.UIManager.GetCharacter(Font, Text[i]);
			glm::vec3 TempPosition = glm::vec3(CurrentPosition.x, CurrentPosition.y + (character.Bearing.y/PixelPerChar) * 2.0f, CurrentPosition.z);
			
			glm::mat4 ModelMatrix = glm::mat4(1.0f);
			ModelMatrix = glm::translate(ModelMatrix, TempPosition)
				* glm::rotate(ModelMatrix, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f))
				* glm::scale(ModelMatrix, glm::vec3(size, 1.0f));

			RenderData.UIBufferIndex = CreateUI(RenderData.UIBufferIndex, color, TextureIndex, character.Coordinate, ModelMatrix);
			CurrentPosition.x += (character.Advance/FontMapWidth) / 2.0f;
			RenderData.UIIndexCount += 6;

			if (RenderData.UIIndexCount >= RenderData.MaxUIComponents * 6)
				FlushScene();
			
		}
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
	void Renderer2D::LoadInFont(const std::string_view& FontLocation, uint32_t Index)
	{
		RenderData.UIManager.LoadFont(FontLocation, Index);
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
	void Renderer2D::DrawUI()
	{
		RenderData.UIVao->Bind();
		RenderData.UIShaders->Use();

		RenderData.UIVbo->Bind();
		RenderData.IndexBuffer->Bind();

		for (int i = 0; i < (int)RenderData.UITextureCount; i++)
			glBindTextureUnit(i, RenderData.UITextureSlots[i]);

		glDrawElements(GL_TRIANGLES, RenderData.UIIndexCount, GL_UNSIGNED_INT, nullptr);

		RenderData.UIIndexCount = 0;
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

		GLsizeiptr UIBufferSize = (uint8_t*)RenderData.UIBufferIndex - (uint8_t*)RenderData.UIBuffer;

		DynamicData UIBufferData;
		UIBufferData.data = RenderData.UIBuffer;
		UIBufferData.Offset = 0;
		UIBufferData.VertexSize = UIBufferSize;

		RenderData.UIVbo->PushData(UIBufferData);

		DrawQuad();
		DrawCircle();
		DrawUI();
	}

	void Renderer2D::ShutDown()
	{
		glDeleteTextures(1, &RenderData.DefaultTexture);

		delete[] RenderData.Buffer;
		delete[] RenderData.CircleBuffer;
		delete[] RenderData.UIBuffer;
		
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
		/*OrthoCameraData CameraData{};
		CameraData.Position = glm::vec3(0.0f, 0.0f, 0.0f);
		CameraData.Front = glm::vec3(0.0f, 0.0f, -1.0f);
		CameraData.Up = glm::vec3(0.0f, 1.0f, 0.0f);

		CameraData.AspectRatio = (float)TGE::Application::GetMainWindow().GetWidth() /
			(float)TGE::Application::GetMainWindow().GetHeight();

		CameraData.ZoomLevel = 1.0f;
		CameraData.CameraSpeed = 1.0f;

		s_RaytracingData.OrthoCamera.SetCam(CameraData);*/

		CameraData CamData;
		CamData.Sensitivity = 0.05f;

		s_RaytracingData.DebuggingCamera.SetCam(CamData);

		TextureData ImageData;
		ImageData.Width  =         s_RaytracingData.ImageWidth;
		ImageData.Height =         s_RaytracingData.ImageHeight;
		ImageData.Type   =		   TextureType::Texture2D;
		ImageData.InternalFormat = TextureFormat::RGBA8;
		ImageData.TextureParamaters =
		{
			{GL_TEXTURE_MIN_FILTER, GL_NEAREST},
			{GL_TEXTURE_MAG_FILTER, GL_NEAREST},
			{GL_TEXTURE_WRAP_S,		GL_CLAMP_TO_EDGE},
			{GL_TEXTURE_WRAP_T,		GL_CLAMP_TO_EDGE}	
		};

		s_RaytracingData.Data = new uint32_t[s_RaytracingData.ImageWidth * s_RaytracingData.ImageHeight];
		for (size_t i = 0; i < s_RaytracingData.ImageWidth * s_RaytracingData.ImageHeight; i++)
			s_RaytracingData.Data[i] = 0xFF000000;
		
		s_RaytracingData.RenderImage = Texture::Generate(s_RaytracingData.Data, ImageData);

		BufferData ShaderStorageData{};
		ShaderStorageData.data = nullptr;
		ShaderStorageData.DrawType = GL_DYNAMIC_DRAW;
		ShaderStorageData.VertexSize = sizeof(Circle) * 100;

		s_RaytracingData.ShaderStorage = OpenGLBuffer::Generate(BufferType::ShaderStorageBuffer, ShaderStorageData);
		s_RaytracingData.ShaderStorage->BindBase(1);

		std::map<GLenum, std::string_view> ComputeShaderLocation =
		{ {GL_COMPUTE_SHADER, "../Resources/Shaders/Defaults/RayTracing/RayTracing.glsl"} };

		s_RaytracingData.ComputeShaders = Shader::Generate(ComputeShaderLocation);

		Attachment attachment(AttachmentType::Color, s_RaytracingData.ImageWidth, s_RaytracingData.ImageHeight);

		/* Triangles will come in later as they are a bit more complicated 
		*  For now a basice pipeline will be established that is fast and efficient
		*  Then triangles will be added in later
		*/
#ifdef TRIANGLE
		
		/* ----Triangle---- */
		glm::vec3 Point1 = { 0.8f, 0.8f, 0.0f };
		glm::vec3 Point2 = { 0.8f, 0.1f, 0.0f };
		glm::vec3 Point3 = { 0.1f, 0.8f, 0.0f };

		glm::vec3 Edge0 = Point2 - Point1;
		glm::vec3 Edge1 = Point3 - Point2;
		glm::vec3 Edge2 = Point1 - Point3;
		glm::vec3 Normal = glm::cross(Edge1, Edge2);
		Normal = glm::normalize(Normal);

		glm::vec3 RayDirection = { 0.0f, 0.0f, -1.0f };

		for (size_t x = 0; x < s_RaytracingData.ImageWidth; x++)
		{
			for (size_t y = 0; y < s_RaytracingData.ImageHeight; y++)
			{
				//cast out rays from each pixel of camera, normalize these coordiantes in a range from -1.0 to 1.0
				glm::vec3 RayOrigin = {
				(2.0f * static_cast<float>(x) / s_RaytracingData.ImageWidth) - 1.0f,
					1.0f - (2.0f * static_cast<float>(y) / s_RaytracingData.ImageHeight), 0.0f
				};


				/*check to see if ray is parralel to triangle(per vectors have dot product of 0)*/
				float NdotRayDirection = glm::dot(Normal,RayDirection);
				if (fabs(NdotRayDirection) <= 0.0f) // almost 0
					continue;

				//Here we are calculating the distance of the triangles point from the plane
				float d = -glm::dot(Normal, Point1);

				//here we calculate the distance along the ray to the point from the ray origin
				float t = -(glm::dot(Normal, RayOrigin) + d) / NdotRayDirection;

				//check to see if triangle is behind ray
				if (t < 0)
					continue;

				//calculate intersection point along the plane(equation for line: Origin + Distance * Direction)
				glm::vec3 IntersectionPoint = RayOrigin + t * RayDirection;

				glm::vec3 C{};

				/*Inside out test*/

				//compute if it on left side of VP0
				glm::vec3 vp0 = IntersectionPoint - Point1;
				C = glm::cross(Edge0, vp0);
				if (glm::dot(Normal,C) < 0) 
					continue; // P is on the right side

				
				//compute if it on left side of VP1
				glm::vec3 vp1 = IntersectionPoint - Point2;
				C = glm::cross(Edge1, vp1);
				if (glm::dot(Normal,C) < 0)  
					continue; // P is on the right side


				//compute if it on left side of VP2
				glm::vec3 vp2 = IntersectionPoint - Point3;
				C = glm::cross(Edge2, vp2);
				if (glm::dot(Normal,C) < 0) 
					continue; // P is on the right side;

				float DistanceFromO = glm::distance(IntersectionPoint, RayOrigin);
				s_RaytracingData.Data[x + y * s_RaytracingData.ImageWidth] = 0xFFFFFF00 - (0xFF0F0FF0 * static_cast<uint32_t>(DistanceFromO));

			}
		}


#endif

	}
	void Raytracing2D::ChangeSampleRate(uint32_t NewSampleRate)
	{
		s_RaytracingData.SampleRate = NewSampleRate;
	}
	void Raytracing2D::Test()
	{
		static glm::vec3 Point1 = { 0.8f, 0.8f, 0.0f };
		static glm::vec3 Point2 = { 0.8f, 0.1f, 0.0f };
		static glm::vec3 Point3 = { 0.1f, 0.8f, 0.0f };

		static glm::vec3 Edge0 = Point2 - Point1;
		static glm::vec3 Edge1 = Point3 - Point2;
		static glm::vec3 Edge2 = Point1 - Point3;
		static glm::vec3 Normal = glm::cross(Edge1, Edge2);
		Normal = glm::normalize(Normal);

		static glm::vec3 RayDirection = { 0.0f, 0.0f, -1.0f };

		/*check to see if ray is parralel to triangle(per vectors have dot product of 0)*/
		static float NdotRayDirection = glm::dot(Normal, RayDirection);

		//Here we are calculating the distance of the triangles point from the plane
		static float d = -glm::dot(Normal, Point1);
		
		std::for_each(std::execution::par, s_RaytracingData.Widths.begin(), s_RaytracingData.Widths.end(), [](int x) 
			{
				std::for_each(std::execution::par, s_RaytracingData.Heights.begin(), s_RaytracingData.Heights.end(), [x](int y) 
					{
						
						//cast out rays from each pixel of camera, normalize these coordiantes in a range from -1.0 to 1.0
						glm::vec3 RayOrigin = {
						(2.0f * static_cast<float>(x) / s_RaytracingData.ImageWidth) - 1.0f,
							1.0f - (2.0f * static_cast<float>(y) / s_RaytracingData.ImageHeight), 0.0f
						};

						//here we calculate the distance along the ray to the point from the ray origin
						float t = -(glm::dot(Normal, RayOrigin) + d) / NdotRayDirection;

						//check to see if triangle is behind ray
						if (t < 0)
							return;

						//calculate intersection point along the plane(equation for line: Origin + Distance * Direction)
						glm::vec3 IntersectionPoint = RayOrigin + t * RayDirection;

						glm::vec3 C{};

						glm::vec3 vp0 = IntersectionPoint - Point1;
						C = glm::cross(Edge0, vp0);
						if (glm::dot(Normal, C) < 0)
							return; // P is on the right side


						glm::vec3 vp1 = IntersectionPoint - Point2;
						C = glm::cross(Edge1, vp1);
						if (glm::dot(Normal, C) < 0)
							return; // P is on the right side


						glm::vec3 vp2 = IntersectionPoint - Point3;
						C = glm::cross(Edge2, vp2);
						if (glm::dot(Normal, C) < 0)
							return; // P is on the right sides
							
					});
			});

	}
	void Raytracing2D::PushCircle(const Circle& CircleData)
	{
		s_RaytracingData.CircleData.push_back(CircleData);
	}
	void Raytracing2D::Trace()
	{

		//s_RaytracingData.OrthoCamera.Update(Application::GetCurrentDeltaSecond());
		s_RaytracingData.DebuggingCamera.Update(Application::GetCurrentDeltaSecond());

		DynamicData ShaderStorageData{};

		ShaderStorageData.data = s_RaytracingData.CircleData.data();
		ShaderStorageData.index = 0;
		ShaderStorageData.Offset = 0;
		ShaderStorageData.VertexSize = s_RaytracingData.CircleData.size() * sizeof(Circle);

		s_RaytracingData.ShaderStorage->PushData(ShaderStorageData);
		s_RaytracingData.ComputeShaders->Use();

		glm::vec3 Pos = s_RaytracingData.DebuggingCamera.GetPosition();

		s_RaytracingData.ComputeShaders->SetUniformInt("NumberOfObjects", s_RaytracingData.CircleData.size());
		s_RaytracingData.ComputeShaders->SetUniformInt("SampleRate", static_cast<int>(s_RaytracingData.SampleRate));
		s_RaytracingData.ComputeShaders->SetUniformFloat3("CameraPosition", Pos.x, Pos.y, Pos.z);
		s_RaytracingData.ComputeShaders->setUniformMat4("InverseView", s_RaytracingData.DebuggingCamera.GetInverseView());
		s_RaytracingData.ComputeShaders->setUniformMat4("InverseProjection", s_RaytracingData.DebuggingCamera.GetInverseProjection());

		s_RaytracingData.RenderImage->BindImage(0);
		s_RaytracingData.ComputeShaders->Compute(std::ceil(s_RaytracingData.ImageWidth / 8),
												 std::ceil(s_RaytracingData.ImageHeight / 4), 1);
		s_RaytracingData.CircleData.clear();
	}
	void Raytracing2D::Shutdown()
	{
		delete[] s_RaytracingData.Data;
	}
	std::shared_ptr<Texture> Raytracing2D::GetRenderImage()
	{
		return s_RaytracingData.RenderImage;
	}
}