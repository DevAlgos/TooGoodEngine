#pragma once

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace TGE
{
	static glm::vec3 QuadVertices[4] = 
	{
		{-1.0f, -1.0f, 0.0f}, // bottom left
		{ 1.0f, -1.0f, 0.0f}, // top left
		{ 1.0f, 1.0f,  0.0f},  // top right
		{-1.0f, 1.0f,  0.0f}   // bottom right

	};

	struct Vertex //What every single Vertex will contain
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec3 Normal;
		glm::vec2 TextureCoordinates;
		float TextureUnit; //Will combine texture unit and material unit in the future
		float MaterialUnit;
		glm::mat4 ModelMatrix; //rotations, translations etc...
	};

	struct CircleVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		float Thickness;
		float MaterialUnit;
		glm::mat4 ModelMatrix;
	};

	struct UIVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::mat4 ModelMatrix;
	};

	struct Material {
		glm::vec3 ambient = { 0.2f, 0.2f, 0.2f };
		float padding1; //these are so the formatting in glsl doesn't missalign the data.
		glm::vec3 diffuse = { 1.0f, 1.0f, 1.0f };
		float padding2;
		glm::vec3 specular = { 1.0f, 1.0f, 1.0f };
		float shininess = 1.0f;

		bool operator==(Material& material)
		{
			return ambient == material.ambient && diffuse == material.diffuse && specular == material.specular && shininess == material.shininess;
		}
	};
	struct LightSource
	{
		glm::vec3 Position;
		float padding1;// again formatting for glsl
		glm::vec3 Color;
		float ID;

		bool operator==(const LightSource& Light)
		{
			return ID == Light.ID;
		}
	};

	struct SpriteSheet
	{
		float TotalWidth = 0.0f;
		float TotalHeight = 0.0f;

		float CellWidth = 0.0f;
		float CellHeight = 0.0f;

		float XIndex = 0.0f;
		float YIndex = 0.0f;
	};

	//All of these sets the vertices and vertex attributes for a quad.
	static Vertex* CreateQuad(Vertex* vertex, float TextureUnit, float MaterialUnit, const glm::mat4& ModelMatrix)
	{

		glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

		vertex->Position = QuadVertices[0];
		vertex->Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex->Color = color;
		vertex->TextureCoordinates = glm::vec2(0.0f, 0.0f);
		vertex->TextureUnit = TextureUnit;
		vertex->MaterialUnit = MaterialUnit;
		vertex->ModelMatrix = ModelMatrix;
		vertex++;


		vertex->Position = QuadVertices[1];
		vertex->Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex->Color = color;
		vertex->TextureCoordinates = glm::vec2(1.0f, 0.0f);
		vertex->TextureUnit = TextureUnit;
		vertex->MaterialUnit = MaterialUnit;
		vertex->ModelMatrix = ModelMatrix;
		vertex++;



		vertex->Position = QuadVertices[2];
		vertex->Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex->Color = color;
		vertex->TextureCoordinates = glm::vec2(1.0f, 1.0f);
		vertex->TextureUnit = TextureUnit;
		vertex->MaterialUnit = MaterialUnit;
		vertex->ModelMatrix = ModelMatrix;
		vertex++;



		vertex->Position = QuadVertices[3];
		vertex->Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex->Color = color;
		vertex->TextureCoordinates = glm::vec2(0.0f, 1.0f);
		vertex->TextureUnit = TextureUnit;
		vertex->MaterialUnit = MaterialUnit;
		vertex->ModelMatrix = ModelMatrix;
		vertex++;


		return vertex;
	}
	static Vertex* CreateQuad(Vertex* vertex, const glm::vec4& color, float MaterialUnit, const glm::mat4& ModelMatrix)
	{
		vertex->Position = QuadVertices[0];
		vertex->Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex->Color = color;
		vertex->TextureCoordinates = glm::vec2(0.0f, 0.0f);
		vertex->TextureUnit = 0.0f;
		vertex->MaterialUnit = MaterialUnit;
		vertex->ModelMatrix = ModelMatrix;
		vertex++;


		vertex->Position = QuadVertices[1];
		vertex->Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex->Color = color;
		vertex->TextureCoordinates = glm::vec2(1.0f, 0.0f);
		vertex->TextureUnit = 0.0f;
		vertex->MaterialUnit = MaterialUnit;
		vertex->ModelMatrix = ModelMatrix;
		vertex++;



		vertex->Position = QuadVertices[2];
		vertex->Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex->Color = color;
		vertex->TextureCoordinates = glm::vec2(1.0f, 1.0f);
		vertex->TextureUnit = 0.0f;
		vertex->MaterialUnit = MaterialUnit;
		vertex->ModelMatrix = ModelMatrix;
		vertex++;



		vertex->Position = QuadVertices[3];
		vertex->Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex->Color = color;
		vertex->TextureCoordinates = glm::vec2(0.0f, 1.0f);
		vertex->TextureUnit = 0.0f;
		vertex->MaterialUnit = MaterialUnit;
		vertex->ModelMatrix = ModelMatrix;
		vertex++;


		return vertex;
	}
	static Vertex* CreateSprite(Vertex* vertex, const SpriteSheet& sheet,
		float TextureUnit, float MaterialUnit, const glm::mat4& ModelMatrix)
	{
		glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };


		float Width = 1.0f / (sheet.TotalWidth / sheet.CellWidth);
		float Height = 1.0f / (sheet.TotalHeight / sheet.CellHeight);

		float x = sheet.XIndex * Width;
		float y = sheet.YIndex * Height;


		vertex->Position = QuadVertices[0];
		vertex->Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex->Color = color;
		vertex->TextureCoordinates = glm::vec2(x, y);
		vertex->TextureUnit = TextureUnit;
		vertex->MaterialUnit = MaterialUnit;
		vertex->ModelMatrix = ModelMatrix;
		vertex++;


		vertex->Position = QuadVertices[1];
		vertex->Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex->Color = color;
		vertex->TextureCoordinates = glm::vec2(x + Width, y);
		vertex->TextureUnit = TextureUnit;
		vertex->MaterialUnit = MaterialUnit;
		vertex->ModelMatrix = ModelMatrix;
		vertex++;



		vertex->Position = QuadVertices[2];
		vertex->Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex->Color = color;
		vertex->TextureCoordinates = glm::vec2(x + Width, y + Height);
		vertex->TextureUnit = TextureUnit;
		vertex->MaterialUnit = MaterialUnit;
		vertex->ModelMatrix = ModelMatrix;
		vertex++;



		vertex->Position = QuadVertices[3];
		vertex->Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex->Color = color;
		vertex->TextureCoordinates = glm::vec2(x, y + Height);
		vertex->TextureUnit = TextureUnit;
		vertex->MaterialUnit = MaterialUnit;
		vertex->ModelMatrix = ModelMatrix;
		vertex++;


		return vertex;
	}

	static CircleVertex* CreateCircle(CircleVertex* vertex, float Thickness, const glm::vec4& color, float MaterialUnit, const glm::mat4& ModelMatrix)
	{
		vertex->Position = QuadVertices[0];
		vertex->Color = color;
		vertex->Thickness = Thickness;
		vertex->MaterialUnit = MaterialUnit;
		vertex->ModelMatrix = ModelMatrix;
		vertex++;

		vertex->Position = QuadVertices[1];
		vertex->Color = color;
		vertex->Thickness = Thickness;
		vertex->MaterialUnit = MaterialUnit;
		vertex->ModelMatrix = ModelMatrix;
		vertex++;



		vertex->Position = QuadVertices[2];
		vertex->Color = color;
		vertex->Thickness = Thickness;
		vertex->MaterialUnit = MaterialUnit;
		vertex->ModelMatrix = ModelMatrix;
		vertex++;

		vertex->Position = QuadVertices[3];
		vertex->Color = color;
		vertex->Thickness = Thickness;
		vertex->MaterialUnit = MaterialUnit;
		vertex->ModelMatrix = ModelMatrix;
		vertex++;

		return vertex;
	}

	static UIVertex* CreateUI(UIVertex* vertex, const glm::vec3& Position, const glm::vec4& Color, const glm::mat4& ModelMatrix)
	{

	}
}