#version 460 core

//per vertex data
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec2 VertexTextureCoordinate;

//per instance data
layout(location = 3) in vec4   Albedo;
layout(location = 4) in vec3   Reflectivity;
layout(location = 5) in float  Metallic;
layout(location = 6) in vec3   Emission;
layout(location = 7) in float  Roughness;
layout(location = 8) in float  TextureIndex;
layout(location = 9) in mat4   Transform;


uniform mat4 ViewProjection;

void main()
{
	gl_Position = ViewProjection * Transform * vec4(VertexPosition, 1.0);
}