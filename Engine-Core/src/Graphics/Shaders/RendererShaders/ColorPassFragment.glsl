#version 460 core

#extension GL_ARB_bindless_texture: require

readonly restrict layout(std430, binding = 0) buffer TextureSlots
{
    sampler2D Samplers[];
} Textures;


in vec3		o_WorldPosition;
in vec3     o_Normal;
in vec2		o_TexCoord;
in vec4     o_Albedo;
in vec3		o_Reflectivity;
in float	o_Metallic;
in vec3     o_Emission;
in float    o_Roughness;
in float    o_TextureIndex;

layout(location = 0) out vec4 Albedo;
layout(location = 1) out vec4 ReflectAndMetallic;
layout(location = 2) out vec4 EmissionAndRoughness;
layout(location = 3) out vec3 Normal;

void main()
{
	int TexID = int(o_TextureIndex + 0.5);
	
	Albedo = o_Albedo * texture(Textures.Samplers[TexID], o_TexCoord);

	ReflectAndMetallic		= vec4(o_Reflectivity, o_Metallic);
	EmissionAndRoughness	= vec4(o_Emission, o_Roughness);
	Normal = o_Normal;
}