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

struct RayPayload
{
    vec3 Color;

    vec2 IntersectionPoint;
    vec2 Normal;
    vec2 RayDirection;
    vec2 RayOrigin;
    float DistanceFromObject;
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

layout(std430, binding = 2) readonly buffer ObjectPositions
{
    vec4 Attributes[]; //3 floats assigned for position, 1 signed for object type
} OPositions;


 
uniform float NumberOfLightSources;
uniform float NumberOfObjects;
uniform vec3 CameraPosition;

/*
a = Bx^2 + By^2
b = (2BxAx - 2DBx) + (2ByAy - 2OBy)
c = (Ax^2 - 2DAx + D^2) + (Ay^2 - 2OAy + O^2) - R^2

Where (D,O) is the origin of the circle.
B is the directional vector of the ray and A is the origin of the ray.*/
vec3 CalculateQuadraticComponents(vec2 RayDirection, vec2 RayOrigin, vec2 CircleCenter, float Radius) //calculates a,b,c for quadratic
{
    float a = dot(RayDirection,RayDirection);
    float b = 
    2 * (RayDirection.x*RayOrigin.x - CircleCenter.x*RayDirection.x) +
    2 * (RayDirection.y*RayOrigin.y - CircleCenter.y*RayDirection.y);

    float c = (RayOrigin.x * RayOrigin.x - 2 * CircleCenter.x * RayOrigin.x + CircleCenter.x*CircleCenter.x) + 
    (RayOrigin.y * RayOrigin.y - 2 * CircleCenter.y * RayOrigin.y + CircleCenter.y*CircleCenter.y) - Radius*Radius;

    return vec3(a,b,c); 
}



RayPayload TraceRay(vec2 RayDirection, vec2 RayOrigin, vec2 ObjectCenter, float Radius)
{
    RayPayload payload;
    payload.RayDirection = RayDirection;
    payload.RayOrigin = RayOrigin;
    
    vec3  Quadratic = CalculateQuadraticComponents(RayDirection, RayOrigin, ObjectCenter, Radius);
    float Discriminant = Quadratic.y*Quadratic.y - 4 * Quadratic.x * Quadratic.z;

    if(Discriminant >= 0)
    {
        float t1 = (-Quadratic.y + sqrt(Discriminant)) / (2.0 * Quadratic.x); // first intersection
        
        payload.IntersectionPoint = payload.RayOrigin + t1 * payload.RayDirection;

        payload.Normal = normalize(payload.IntersectionPoint - ObjectCenter);
        payload.DistanceFromObject = distance(payload.IntersectionPoint ,RayOrigin);
    } else
    {
        payload.DistanceFromObject = -1.0;
    }

    return payload;
}


vec3 ShadePixel(RayPayload Payload, vec3 LightColor)
{
    vec3 Color = vec3(1.0);

    float constantAttenuation = 1.0;
    float linearAttenuation = 0.1; 
    float quadraticAttenuation = 0.01; 
    float Attenuation = 1.0 / 
            (constantAttenuation + linearAttenuation * 
            Payload.DistanceFromObject + quadraticAttenuation * 
            Payload.DistanceFromObject * Payload.DistanceFromObject);



    float DiffuseStrength = max(dot(vec3(Payload.Normal,1.0), vec3(Payload.RayDirection,1.0)),0.0);

    vec3  Ambient =      MatSlots.Materials[int(MaterialID)].Ambient;
    vec3  Diffuse =      MatSlots.Materials[int(MaterialID)].Diffuse;
    vec3  Specular =     MatSlots.Materials[int(MaterialID)].Specular;
    float Shininess =    MatSlots.Materials[int(MaterialID)].SpecularStrength;

    vec3 FinalDiffuse = DiffuseStrength * Diffuse * Attenuation;

    vec3 ViewDir =    normalize(CameraPosition - WorldCoordinates);
    vec3 ReflectDir = reflect(vec3(-Payload.RayDirection, 0.0), vec3(Payload.Normal, 0.0));  

    float Spec = max(pow(dot(ViewDir, ReflectDir), Shininess), 0.0);
    vec3  FinalSpecular = Spec * Specular;  

    vec3 phong = (Ambient + FinalDiffuse + FinalSpecular) * LightColor;

    Color = phong; 
    
    return Color;
}


vec3 TraceAllRays()
{   
   vec3 CircleColor = vec3(1.0);

    vec2 Rays[4];


    Rays[0] = vec2(0.0, 1.0);
    Rays[1] = vec2(1.0, 0.0);
    Rays[2] = vec2(1.0, 1.0);
    Rays[3] = vec2(-1.0, 1.0);


    for(int LightIndex = 0; LightIndex < NumberOfLightSources; LightIndex++)
    {
        vec2 LightPosition = Lights.LightSrc[LightIndex].Position.xy;
        vec3 LightColor    = Lights.LightSrc[LightIndex].Color;

        for(int ObjectIndex = 0; ObjectIndex < NumberOfObjects; ObjectIndex++)
        {
            vec2 ObjectCenter = OPositions.Attributes[ObjectIndex].xy;
            float Radius = OPositions.Attributes[ObjectIndex].z;
            
            for(int RayIndex = 0; RayIndex < 4; RayIndex++)
            {
                RayPayload payload = TraceRay(Rays[RayIndex], LightPosition, ObjectCenter, Radius);
                payload.Color = vec3(1.0);

                if(ObjectCenter == WorldCoordinates.xy) //We have hit target object, shade pixel 
                {
                    if(payload.DistanceFromObject >= 0)
                      payload.Color = ShadePixel(payload, LightColor);
                } //soon to add miss function to calculate bounces
           

                CircleColor *= payload.Color; //missed
            }
            
        }
    }
    


   return CircleColor;
}

void main()
{
    float fade = 0.005;
    float distancef = 1.0 - length(Position.xy);
    
    int nLightSources = int(NumberOfLightSources);
    int nObjects = int(NumberOfObjects);
    
    vec3 Light = vec3(1.0f, 1.0f, 1.0f);
   
    Light = TraceAllRays();
  
    distancef = smoothstep(0.0, fade, distancef);
    distancef *= smoothstep(Thickness + fade, Thickness, distancef);

    FragColor.w = distancef;
    FragColor.rgb = Color.rgb;
    FragColor.rgb *= Light;
   
}