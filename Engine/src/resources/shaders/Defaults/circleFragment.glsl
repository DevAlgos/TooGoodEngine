#version 460 core

struct LightSource
{
    vec3 Position;
    vec3 Color;
    float ID;
};

struct Material
{
    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
    float SpecularStrength;
};

out vec4 FragColor;

in vec3  Position;
in vec4  Color;
in vec3  Normal;
in float Thickness;
in float MaterialID;


layout(std140, binding = 0) uniform MaterialSlots
{
    Material Materials[32];
} MatSlots;


layout(std140, binding = 1) uniform LightSources
{
    LightSource LightSrc[6];
} Lights;


uniform float NumberOfLightSources;

void main()
{
    float fade = 0.005;
    
   float nLightSources = NumberOfLightSources;
  
   float distancef = 1.0 - length(Position.xy);
   distancef = smoothstep(0.0, fade, distancef);
   distancef *= smoothstep(Thickness + fade, Thickness, distancef);

   FragColor.w = distancef;
   FragColor.rgb = Color.rgb;
   
}