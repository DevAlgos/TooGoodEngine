#version 460 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in float inRadius;
layout(location = 4) in float inThickness;
layout(location = 5) in float inTexID;
layout(location = 6) in float inMaterialID;
layout(location = 7) in mat4 inModelMatrix;

uniform mat4 view;
uniform mat4 projection;



out vec3 Position;
out vec4 Color;
out vec3 Normal;
out float Radius;
out float Thickness;
out float TexID;
out float MaterialID;


void main()
{
	  gl_Position =  projection * view * inModelMatrix * vec4(inPosition, 1.0);
	 
	  Position = vec3(inModelMatrix * vec4(inPosition, 1.0));

	  Color =        inColor;
	  Normal =       inNormal;
	  Radius =       inRadius;
	  Thickness =    inThickness;
	  TexID =		 inTexID;
	  MaterialID =   inMaterialID;
}