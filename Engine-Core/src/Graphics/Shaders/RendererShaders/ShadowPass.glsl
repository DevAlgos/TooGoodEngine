#version 460 core

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D ColorBuffer;
layout (binding = 1) uniform sampler2D DepthBuffer;

layout(rgba8, binding = 2) writeonly uniform image2D ShadowBuffer;
layout(rgba32f, binding = 3) uniform image2D NormalBuffer;


#define EPSILON 1.192092896e-07f
#define FLOAT_MAX 3.402823466e+38f

#define MAX_STACK_SIZE 1000
#define SHADOW_BIAS 0.05

uniform mat4 InverseProjection;
uniform mat4 InverseView;

uniform int nLightSources;

struct DirectionalLightSource
{
    vec4 Direction;
    vec4 ColorAndIntensity;
};

struct AABB
{
	vec4 MinValue;
	vec4 MaxValue;
};

struct Triangle
{
	vec4 Vertices[3];
	vec4 Normal;
    vec4 Centroid;
};

struct BVHNode
{
    AABB BoundingBox;
    int LeftNode;
	int RightNode;
    int NumberOfPrims;
    bool IsLeaf;
    int Primitive[100];
};

readonly layout(std430, binding = 3) buffer TriangleData
{
    Triangle data[];

} Triangles;

readonly layout(std430, binding = 4) buffer AccelerationStructure
{
	BVHNode nodes[];

} BVHStructure;

readonly layout(std430, binding = 5) buffer LightSources
{
    DirectionalLightSource Sources[];
} LightData;

struct RayPayload 
{
    vec3 Origin;
    vec3 Direction;

    float t;

	bool InLight;
    bool Valid;
};

float IntersectAABB(in RayPayload Payload, in AABB box)
{
    vec3 tMin = (box.MinValue.xyz - Payload.Origin.xyz) / Payload.Direction.xyz;
    vec3 tMax = (box.MaxValue.xyz - Payload.Origin.xyz) / Payload.Direction.xyz;

    vec3 tMinOrder = min(tMin, tMax);
    vec3 tMaxOrder = max(tMin, tMax);

    float t0 = max(max(tMinOrder.x, tMinOrder.y), tMinOrder.z);
    float t1 = min(min(tMaxOrder.x, tMaxOrder.y), tMaxOrder.z);

    float Distance = (t0 <= t1) ? t0 : FLOAT_MAX;

    return Distance;
}

bool FastTriangleIntersect(in RayPayload Payload, int TriangleIndex)
{
    Triangle triangle = Triangles.data[TriangleIndex];

    vec3 Vertex1 = vec3(triangle.Vertices[0]); // A
    vec3 Vertex2 = vec3(triangle.Vertices[1]); // B
    vec3 Vertex3 = vec3(triangle.Vertices[2]); // C

    vec3 Normal = vec3(triangle.Normal);

    float Denominator = dot(Payload.Direction, Normal);

    if (Denominator <= EPSILON)
        return false;

    float t = dot(Vertex1 - Payload.Origin, Normal) / Denominator;

    if (t < 0.0)
        return false;

    vec3 Ray = Payload.Origin + Payload.Direction * t;
    vec3 e1 = Vertex2 - Vertex1;
    vec3 e2 = Vertex3 - Vertex1;
   
    vec3 h = cross(Payload.Direction, e2);
    float a = dot(e1, h);
   
    if (abs(a) <= EPSILON)
       return false;
   
    vec3 s = Ray - Vertex1;
   
    float w1 = dot(s, h) / a;
   
    if (w1 < 0.0 || w1 > 1.0)
       return false;
   
    vec3 q = cross(s, e1);
    float w2 = dot(Payload.Direction, q) / a;
   
    if (w2 < 0.0 || w1 + w2 > 1.0)
       return false;
   
    return true;
}




bool IntersectTriangle(in RayPayload Payload, int TriangleIndex)
{
    Triangle triangle = Triangles.data[TriangleIndex];

    vec3 Vertex1 = vec3(triangle.Vertices[0]);
    vec3 Vertex2 = vec3(triangle.Vertices[1]);
    vec3 Vertex3 = vec3(triangle.Vertices[2]);

    vec3 Normal = vec3(triangle.Normal);

    // Check if the ray is parallel to the triangle
    float NdotRayDirection = dot(Normal, Payload.Direction);
    if (abs(NdotRayDirection) <= EPSILON)
        return false;

    float d = -dot(Normal, Vertex1);

    // Compute the intersection distance
    float t = -(dot(Normal, Payload.Origin) + d) / NdotRayDirection;

    // Check if the intersection point is behind the ray's origin
    if (t < 0.0)
        return false;

    // Compute the intersection point
    vec3 IntersectionPoint = Payload.Origin + t * Payload.Direction;

    vec3 C0 = cross(Vertex2 - Vertex1, IntersectionPoint - Vertex1);

    if(dot(Normal, C0) < 0.0)
        return false;

    vec3 C1 = cross(Vertex3 - Vertex2, IntersectionPoint - Vertex2);

    if(dot(Normal, C1) < 0.0)
        return false;

    vec3 C2 = cross(Vertex1 - Vertex3, IntersectionPoint - Vertex3);

    if(dot(Normal, C2) < 0.0)
        return false;

    return true;
}

void TraceRay(inout RayPayload Payload)
{    
    int Stack[MAX_STACK_SIZE];
    int StackPtr = 0;

    Stack[StackPtr++] = 0;

    while (StackPtr > 0)
    {
        int CurrentNode = Stack[--StackPtr];

        if (BVHStructure.nodes[CurrentNode].IsLeaf)
        {
            for (int i = 0; i < BVHStructure.nodes[CurrentNode].NumberOfPrims; i++)
            {
                if (FastTriangleIntersect(Payload, BVHStructure.nodes[CurrentNode].Primitive[i]))
                {
                    Payload.InLight = false;
                    return; //ray is occluded we can break
                }
            }
        }
        else
        {
            int Child1 = BVHStructure.nodes[CurrentNode].LeftNode;
            int Child2 = BVHStructure.nodes[CurrentNode].RightNode;

            float Distance1 = FLOAT_MAX;
            float Distance2 = FLOAT_MAX;

            if (Child1 != -1)
                Distance1 = IntersectAABB(Payload, BVHStructure.nodes[Child1].BoundingBox);
            

            if (Child2 != -1)
                Distance2 = IntersectAABB(Payload, BVHStructure.nodes[Child2].BoundingBox);
            

            if (Distance1 != FLOAT_MAX && Distance2 != FLOAT_MAX)
            {
                if (Distance1 < Distance2)
                {
                    Stack[StackPtr++] = Child1;
                    Stack[StackPtr++] = Child2;
                }
                else if (Distance2 < Distance1)
                {
                    Stack[StackPtr++] = Child2;
                    Stack[StackPtr++] = Child1;
                }
                else 
                {
                    Stack[StackPtr++] = Child1;
                    Stack[StackPtr++] = Child2;
                }
            }
            else if (Distance1 != FLOAT_MAX)
            {
                Stack[StackPtr++] = Child1;
            }
            else if (Distance2 != FLOAT_MAX)
            {
                Stack[StackPtr++] = Child2;
            }
           
        }
    }
}

RayPayload DispatchShadowRay(in ivec2 Coordinate, in vec3 LightDirection)
{
	RayPayload Payload;
    Payload.Valid = true;
	Payload.InLight = true;

    vec2 ImageSize = imageSize(ColorBuffer);

    vec2 NormalizedCoord = (vec2(Coordinate)) / ImageSize;
    float DepthValue = texelFetch(DepthBuffer, Coordinate, 0).r;

    if(DepthValue == 1.0)
        return Payload;
        
    vec4 WorldSpace = InverseView * InverseProjection * vec4(NormalizedCoord * 2.0 - 1.0, 
                                                             DepthValue * 2.0 - 1.0, 1.0);
    

    WorldSpace /= WorldSpace.w;


    vec3 Normal = imageLoad(NormalBuffer, Coordinate).rgb;

	Payload.Origin = WorldSpace.xyz + Normal * SHADOW_BIAS;

    Payload.Direction = normalize(-LightDirection);

	TraceRay(Payload);

	return Payload;
}

void main()
{
	ivec2 Coordinate = ivec2(gl_GlobalInvocationID.xy);

    bool InShadow = true;

    for(int i = 0; i < nLightSources; i++)
    {
        vec3 LightDirection = LightData.Sources[i].Direction.xyz;

        RayPayload Payload = DispatchShadowRay(Coordinate, LightDirection);
        
        if(Payload.InLight)
        {
            InShadow = false;
            break;
        }
    }

     float ShadowVal = 0.0;

     if(InShadow)
        ShadowVal = 0.5;
    
     imageStore(ShadowBuffer, Coordinate, vec4(ShadowVal));

}

