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

out vec3  Position;
out vec4  Color;
out vec3  Normal;
out float Radius;
out float Thickness;
out float TexID;
out float MaterialID;


layout(std140, binding = 0) uniform MaterialSlots
{
    Material Materials[32];
} MatSlots;


layout(std140, binding = 1) uniform LightSources
{
    LightSource LightSrc[6];
} Lights;


uniform sampler2D samplerTextures[32];
uniform float NumberOfLightSources;

uniform vec2 ViewportResolution;

void main()
{
    float fade = 0.005;
    

    int ID = int(TexID);
    int MatID = int(MaterialID);
    int NumbOfLightSrc = int(NumberOfLightSources);

    vec4 t = vec4(1.0);

    vec3 ambient = MatSlots.Materials[MatID].Ambient;
    vec3 diffuse = MatSlots.Materials[MatID].Diffuse;
    vec3 specular = MatSlots.Materials[MatID].Specular;
    float shininess = MatSlots.Materials[MatID].SpecularStrength;
    
    vec3 TotalDiffuse = vec3(0.0);
    vec3 FinalLightColor = vec3(0.0);
    
    for(int i = 0; i < NumbOfLightSrc; i++)
    {
        vec3 LightDistance = normalize(Lights.LightSrc[i].Position - Position);
        float Attenuation = float(1/0.2 + LightDistance*LightDistance);
    
        vec3 FinalDiffuse = vec3(dot(Normal, LightDistance));
        FinalDiffuse *= diffuse * Attenuation;
        TotalDiffuse += FinalDiffuse;

        vec3 LightContribution = Lights.LightSrc[i].Color * FinalDiffuse;
        FinalLightColor += LightContribution;
    }
    
   vec2 uv = vec2(Position) / ViewportResolution * 2.0 - 1.0;
   float distancee = 1.0 - length(uv);
   vec3 CircleColor = vec3(smoothstep(0.0, fade, distancee));
   CircleColor *= vec3(smoothstep(Thickness + fade, Thickness, distancee));


    switch (ID)
    {
        case 0:
            t = texture(samplerTextures[0], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 1:
            t = texture(samplerTextures[1], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 2:
            t = texture(samplerTextures[2], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 3:
            t = texture(samplerTextures[3], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 4:
            t = texture(samplerTextures[4], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 5:
            t = texture(samplerTextures[5], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 6:
            t = texture(samplerTextures[6], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 7:
            t = texture(samplerTextures[7], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 8:
            t = texture(samplerTextures[8], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 9:
            t = texture(samplerTextures[9], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 10:
            t = texture(samplerTextures[10], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 11:
            t = texture(samplerTextures[11], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 12:
            t = texture(samplerTextures[12], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 13:
            t = texture(samplerTextures[13], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 14:
            t = texture(samplerTextures[14], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 15:
            t = texture(samplerTextures[15], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 16:
            t = texture(samplerTextures[16], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 17:
            t = texture(samplerTextures[17], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 18:
            t = texture(samplerTextures[18], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 19:
            t = texture(samplerTextures[19], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 20:
            t = texture(samplerTextures[20], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 21:
            t = texture(samplerTextures[21], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 22:
            t = texture(samplerTextures[22], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 23:
            t = texture(samplerTextures[23], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 24:
            t = texture(samplerTextures[24], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 25:
            t = texture(samplerTextures[25], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 26:
            t = texture(samplerTextures[26], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 27:
            t = texture(samplerTextures[27], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 28:
            t = texture(samplerTextures[28], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 29:
            t = texture(samplerTextures[29], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 30:
            t = texture(samplerTextures[30], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        case 31:
            t = texture(samplerTextures[31], vec2(0.5 + uv.x / 2.0, 0.5 - uv.y / 2.0));
            break;
        default:
            break;
    }
   
   vec3 TotalColor = ambient + FinalLightColor;

   vec4 FinalColor = (t * Color);
   FinalColor *= vec4(TotalColor,1.0);

   FragColor = vec4(1.0);
}