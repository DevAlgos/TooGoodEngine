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

out vec3	o_WorldPosition;
out vec3    o_Normal;
out vec2	o_TexCoord;
out vec4    o_Albedo;
out vec3	o_Reflectivity;
out float	o_Metallic;
out vec3    o_Emission;
out float   o_Roughness;
out float   o_TextureIndex;


uniform mat4 ViewProjection;

void main()
{
    vec4 WorldPosition = Transform * vec4(VertexPosition, 1.0);

	mat3 InvTrans = mat3(inverse(transpose(Transform)));

	o_Normal =   normalize(InvTrans * VertexNormal.xyz);
	o_TexCoord = VertexTextureCoordinate;

	gl_Position = ViewProjection * WorldPosition;

	o_WorldPosition = WorldPosition.xyz;
	o_Albedo = Albedo;
	o_Reflectivity = Reflectivity;
	o_Metallic = Metallic;
	o_Emission = Emission;
	o_Roughness = Roughness;
	o_TextureIndex = TextureIndex;
}