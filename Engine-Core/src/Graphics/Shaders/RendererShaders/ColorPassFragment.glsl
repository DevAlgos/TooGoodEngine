#version 460 core

#extension GL_ARB_bindless_texture: require

out vec4 FragColor;

readonly restrict layout(std140, binding = 0) buffer TextureSlots
{
    sampler2D Samplers[];
} Textures;

void main()
{
	FragColor = vec4(0.8, 0.4, 1.0, 1.0);
}