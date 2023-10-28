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

in vec4 Color;
in vec3 Normal;
in vec2 TexCoord;
in float TexID;
in float MaterialID;


in vec3 LightPos;
in vec3 Position;


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

void main()
{
    int ID = int(TexID);
    int MatID = int(MaterialID);
    int NumbOfLightSrc = int(NumberOfLightSources);

    vec4 t = vec4(1.0);

    vec3 ambient = MatSlots.Materials[MatID].Ambient;
    vec3 diffuse = MatSlots.Materials[MatID].Diffuse;
    vec3 specular = MatSlots.Materials[MatID].Specular;
    float shininess = MatSlots.Materials[MatID].SpecularStrength;
    vec3 normal = Normal;
    
    vec3 TotalDiffuse = vec3(0.0);
    vec3 FinalLightColor = vec3(0.0);
    
  

    for(int i = 0; i < NumbOfLightSrc; i++)
    {
        vec3 LightDistance = normalize(Lights.LightSrc[i].Position - Position);
        float Attenuation = float(1/0.2 + LightDistance*LightDistance);
    
        vec3 FinalDiffuse = vec3(dot(normal, LightDistance));
        FinalDiffuse *= diffuse * Attenuation;
        TotalDiffuse += FinalDiffuse;

        vec3 LightContribution = Lights.LightSrc[i].Color * FinalDiffuse;
        FinalLightColor += LightContribution;
    }
    


    switch (ID)
    {
        case 0:
            t = texture(samplerTextures[0], TexCoord);
            break;
        case 1:
            t = texture(samplerTextures[1], TexCoord);
            break;
        case 2:
            t = texture(samplerTextures[2], TexCoord);
            break;
        case 3:
            t = texture(samplerTextures[3], TexCoord);
            break;
        case 4:
            t = texture(samplerTextures[4], TexCoord);
            break;
        case 5:
            t = texture(samplerTextures[5], TexCoord);
            break;
        case 6:
            t = texture(samplerTextures[6], TexCoord);
            break;
        case 7:
            t = texture(samplerTextures[7], TexCoord);
            break;
        case 8:
            t = texture(samplerTextures[8], TexCoord);
            break;
        case 9:
            t = texture(samplerTextures[9], TexCoord);
            break;
        case 10:
            t = texture(samplerTextures[10], TexCoord);
            break;
        case 11:
            t = texture(samplerTextures[11], TexCoord);
            break;
        case 12:
            t = texture(samplerTextures[12], TexCoord);
            break;
        case 13:
            t = texture(samplerTextures[13], TexCoord);
            break;
        case 14:
            t = texture(samplerTextures[14], TexCoord);
            break;
        case 15:
            t = texture(samplerTextures[15], TexCoord);
            break;
        case 16:
            t = texture(samplerTextures[16], TexCoord);
            break;
        case 17:
            t = texture(samplerTextures[17], TexCoord);
            break;
        case 18:
            t = texture(samplerTextures[18], TexCoord);
            break;
        case 19:
            t = texture(samplerTextures[19], TexCoord);
            break;
        case 20:
            t = texture(samplerTextures[20], TexCoord);
            break;
        case 21:
            t = texture(samplerTextures[21], TexCoord);
            break;
        case 22:
            t = texture(samplerTextures[22], TexCoord);
            break;
        case 23:
            t = texture(samplerTextures[23], TexCoord);
            break;
        case 24:
            t = texture(samplerTextures[24], TexCoord);
            break;
        case 25:
            t = texture(samplerTextures[25], TexCoord);
            break;
        case 26:
            t = texture(samplerTextures[26], TexCoord);
            break;
        case 27:
            t = texture(samplerTextures[27], TexCoord);
            break;
        case 28:
            t = texture(samplerTextures[28], TexCoord);
            break;
        case 29:
            t = texture(samplerTextures[29], TexCoord);
            break;
        case 30:
            t = texture(samplerTextures[30], TexCoord);
            break;
        case 31:
            t = texture(samplerTextures[31], TexCoord);
            break;
        default:
            break;
    }

   
   vec3 TotalColor = ambient + FinalLightColor;

   vec4 FinalColor = (t * Color);
   FinalColor *= vec4(TotalColor,1.0);

   FragColor = FinalColor;
}
