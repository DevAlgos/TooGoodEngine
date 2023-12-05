#version 460 core

//position color model

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in float inTexID;
layout(location = 4) in mat4 inModelMatrix;

/*
glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TextureCoordinates;
		float	  TextureUnit;
		glm::mat4 ModelMatrix;
*/

uniform mat4 view;
uniform mat4 projection;

out vec4 oColor;
out float oTexID;
out vec2 TexCoord;

void main()
{
	gl_Position = projection * view * inModelMatrix * vec4(inPosition, 1.0);
	oColor = inColor;
	oTexID = inTexID;
	TexCoord = inTexCoord;
}