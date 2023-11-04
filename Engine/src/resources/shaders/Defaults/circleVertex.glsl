#version 460 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in float inThickness;
layout(location = 3) in float inMaterialID;
layout(location = 4) in mat4 inModelMatrix;

uniform mat4 view;
uniform mat4 projection;

out vec3 Position;
out vec3 WorldCoordinates;
out vec4 Color;
out float Thickness;
out float MaterialID;


void main()
{
	gl_Position =  projection * view * inModelMatrix * vec4(inPosition, 1.0);
	 
	WorldCoordinates = vec3(inModelMatrix * vec4(Position, 1.0));
	Position = inPosition;

    Color =  inColor;
    Thickness = inThickness;
    MaterialID = inMaterialID;
}