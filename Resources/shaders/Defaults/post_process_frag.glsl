#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    vec4 col = texture(screenTexture, TexCoords);


    FragColor = col;
}