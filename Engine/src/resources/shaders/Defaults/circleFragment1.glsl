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

/*
Object Type      Corresponding float 
Quad             0.0
Circle           1.0
*/
 
uniform float NumberOfLightSources;
uniform float NumberOfObjects;

const float MAX_FLOAT = 100;

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

    float constantAttenuation = 0.1;
    float linearAttenuation = 0.1; 
    float quadraticAttenuation = 0.05; 
    float Attenuation = 1.0 / 
            (constantAttenuation + linearAttenuation * 
            Payload.DistanceFromObject + quadraticAttenuation * 
            Payload.DistanceFromObject * Payload.DistanceFromObject);

    vec3 FinalDiffuse = vec3(dot(vec3(Payload.Normal,1.0), vec3(Payload.RayDirection,1.0)));

    FinalDiffuse *= MatSlots.Materials[0].Diffuse * Attenuation;

    Color *= FinalDiffuse * LightColor; 
    
    return Color;
}


vec3 TraceAllRays()
{   
   vec3 CircleColor = vec3(1.0);

    vec2 Rays[1000];


    for (int i = 0; i < 1000; i++) {
        float angle = 2.0 * 3.14 * float(i) / 1000.0; // Spread rays evenly around the unit circle
        vec2 direction = vec2(cos(angle), sin(angle)); // Calculate the direction using trigonometric functions
        Rays[i] = normalize(direction); // Normalize the direction to ensure it's a unit vector
    }



    for(int LightIndex = 0; LightIndex < NumberOfLightSources; LightIndex++)
    {
        vec2 LightPosition = Lights.LightSrc[LightIndex].Position.xy;
        vec3 LightColor    = Lights.LightSrc[LightIndex].Color;

        for(int ObjectIndex = 0; ObjectIndex < NumberOfObjects; ObjectIndex++)
        {
            vec2 ObjectCenter = OPositions.Attributes[ObjectIndex].xy;
            float Radius = OPositions.Attributes[ObjectIndex].z;
            
            for(int RayIndex = 0; RayIndex < 1000; RayIndex++)
            {
                RayPayload payload = TraceRay(Rays[RayIndex], LightPosition, ObjectCenter, Radius);
                payload.Color = vec3(1.0);

                if(ObjectCenter == WorldCoordinates.xy)
                {
                    if(payload.DistanceFromObject >= 0)
                      payload.Color = ShadePixel(payload, LightColor);
                }
           

                CircleColor *= payload.Color;
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
    
    vec3 Lightening = vec3(1.0f, 1.0f, 1.0f);
   
   //artificial ray for testing
    vec2 RayDirection = normalize(vec2(1.0,1.0));
    vec2 RayOrigin = vec2(Lights.LightSrc[0].Position.xy);

    //circle is positioned at 2.0f, 1.0f, with a radius of 1.0f

    float xCoord = WorldCoordinates.x;
    float yCoord = WorldCoordinates.y;

    vec2 CircleCenter = vec2(xCoord,yCoord);

    //vec3 Quadratic = CalculateQuadraticComponents(RayDirection, RayOrigin, CircleCenter);


    Lightening = TraceAllRays();
  

    distancef = smoothstep(0.0, fade, distancef);
    distancef *= smoothstep(Thickness + fade, Thickness, distancef);

    FragColor.w = distancef;
    FragColor.rgb = Color.rgb;
    FragColor.rgb *= Lightening;
   
}