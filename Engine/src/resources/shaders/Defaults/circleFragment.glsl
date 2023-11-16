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
in vec3  WorldCoordinates;
in vec4  Color;
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
uniform vec3 CameraPosition;

void main()
{
    float fade = 0.005;
    float distancef = 1.0 - length(Position.xy);
    
    vec3 Light = vec3(1.0f, 1.0f, 1.0f);

    int MatID = int(MaterialID);
    int NumbOfLightSrc = int(NumberOfLightSources);

    vec3 Ambient =    MatSlots.Materials[MatID].Ambient;
    vec3 Diffuse =    MatSlots.Materials[MatID].Diffuse;
    vec3 Specular =   MatSlots.Materials[MatID].Specular;
    float Shininess = MatSlots.Materials[MatID].SpecularStrength;

    vec3 FinalLightColor = vec3(0.0);
    
    vec3 Normal = vec3(0.0, 0.0, 1.0);
  
    vec3 eye = CameraPosition;

    for(int i = 0; i < NumbOfLightSrc; i++)
    {
        vec3 LightDirection = normalize(Lights.LightSrc[i].Position - WorldCoordinates);
        float Attenuation = float(1/0.2 + LightDirection*LightDirection);
    
        vec3 FinalDiffuse = vec3(dot(Normal, LightDirection));
        FinalDiffuse *= Diffuse * Attenuation;

        vec3 ViewDir = normalize(CameraPosition - WorldCoordinates);
        vec3 ReflectDir = reflect(-LightDirection, Normal);
    
        float Spec = pow(max(dot(ViewDir, ReflectDir), 0.0), Shininess);
        vec3 FinalSpecular = Spec * Specular;

        vec3 LightContribution = Lights.LightSrc[i].Color * 
        (Ambient + FinalDiffuse + FinalSpecular);

        FinalLightColor += LightContribution;
    }
  
    distancef = smoothstep(0.0, fade, distancef);
    distancef *= smoothstep(Thickness + fade, Thickness, distancef);

    FragColor.w = distancef;
    FragColor.rgb = Color.rgb;
    FragColor.rgb *= FinalLightColor;
   
}
