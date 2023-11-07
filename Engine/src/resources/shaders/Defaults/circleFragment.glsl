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

struct Object
{
    vec3 Position;
    float Type; //0 is square, 1 is circle etc...
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


/*
a = Bx^2 + By^2
b = (2BxAx - 2DBx) + (2ByAy - 2OBy)
c = (Ax^2 - 2DAx + D^2) + (Ay^2 - 2OAy + O^2) - R^2

Where (D,O) is the origin of the circle.
B is the directional vector of the ray and A is the origin of the ray.*/

bool TraceRay(float a, float b,float c)
{
    return b*b - 4 * a * c >= 0; //ray will successfully hit if discriminant is >=0
}


vec3 CalculateQuadraticComponents(vec2 RayDirection, vec2 RayOrigin, vec2 CircleCenter) //calculates a,b,c for quadratic
{
    float a = dot(RayDirection,RayDirection);
    float b = 
    2 * (RayDirection.x*RayOrigin.x - CircleCenter.x*RayDirection.x) +
    2 * (RayDirection.y*RayOrigin.y - CircleCenter.y*RayDirection.y);

    float c = (RayOrigin.x * RayOrigin.x - 2 * CircleCenter.x * RayOrigin.x + CircleCenter.x*CircleCenter.x) + 
    (RayOrigin.y * RayOrigin.y - 2 * CircleCenter.y * RayOrigin.y + CircleCenter.y*CircleCenter.y) - CircleCenter.y;

    return vec3(a,b,c); 
}

vec3 CalculateIntersectionColor(float a, float b, float c, vec2 RayOrigin, vec2 RayDirection, vec3 Color, 
                                    vec2 CircleCenter)
{
    vec3 IntersectionColor = vec3(0.0); //should be ambient by default

    float discriminant = b*b - 4 * a * c;
    if(discriminant >= 0)
    {
         // Calculate the intersection points
        float t1 = (-b + sqrt(discriminant)) / (2.0 * a);
        float t2 = (-b - sqrt(discriminant)) / (2.0 * a);

        vec2 IntersectionPoint1 = RayOrigin + t1 * RayDirection;
        vec2 IntersectionPoint2 = RayOrigin + t2 * RayDirection;

        float LightDistance1 = distance(IntersectionPoint1, RayOrigin);
        float LightDistance2 = distance(IntersectionPoint2, RayOrigin);

        float LightDistance = min(LightDistance1, LightDistance2);

        float Attenuation = 1.0; 

        if(LightDistance1 >= LightDistance2)
        {
            float constantAttenuation = 1.0;
            float linearAttenuation = 0.1; 
            float quadraticAttenuation = 0.05; 
            Attenuation = 1.0 / (
            constantAttenuation + linearAttenuation * 
            LightDistance + quadraticAttenuation * 
            LightDistance * LightDistance);

            vec2 Normal = normalize(IntersectionPoint1 - CircleCenter);
            vec3 LightDirection = normalize(Lights.LightSrc[0].Position - vec3(RayOrigin,0.0));

            vec3 FinalDiffuse = vec3(dot(vec3(Normal,1.0), LightDirection));

            FinalDiffuse *= MatSlots.Materials[0].Diffuse * Attenuation;


            IntersectionColor = Color * FinalDiffuse; 
        
        }

       
            
        
   }


    return IntersectionColor;
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

    vec3 Quadratic = CalculateQuadraticComponents(RayDirection, RayOrigin, CircleCenter);


    Lightening = CalculateIntersectionColor(Quadratic.x,Quadratic.y,Quadratic.z,RayOrigin,RayDirection, 
        Lights.LightSrc[0].Color, CircleCenter);

   

  

    distancef = smoothstep(0.0, fade, distancef);
    distancef *= smoothstep(Thickness + fade, Thickness, distancef);

    FragColor.w = distancef;
    FragColor.rgb = Color.rgb;
    FragColor.rgb *= Lightening;
   
}