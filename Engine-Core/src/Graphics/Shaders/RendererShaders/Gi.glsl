#version 460 core

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D ColorBuffer;
layout(rgba32f, binding = 1) uniform image2D ReflectAndMetallicBuffer;
layout(rgba32f, binding = 2) uniform image2D EmissionAndRoughnessBuffer;
layout(rgba32f, binding = 3) uniform image2D NormalBuffer;

layout(binding = 4) uniform sampler2D DepthBuffer;

layout(rgba32f, binding = 5) uniform image2D AccumulationBuffer;

#define EPSILON 1.192092896e-07f
#define UINT_MAX 0xffffffffu
#define FLOAT_MAX 3.402823466e+38f

#define MAX_BOUNCES 3 //TODO: make uniform
#define MAX_STACK_SIZE 1000

uniform mat4 InverseProjection;
uniform mat4 InverseView;

uniform mat4 ViewProjection;
uniform int FrameIndex;

uint CurrentSeed;

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

struct RayPayload 
{
    vec3 Origin;
    vec3 Direction;

    vec3 Normal;
    vec3 IntersectionPoint;

    float Distance;
	int ClosestTriangleIndex;
};

readonly layout(std430, binding = 3) buffer TriangleData
{
    Triangle data[];

} Triangles;

readonly layout(std430, binding = 4) buffer AccelerationStructure
{
	BVHNode nodes[];
} BVH;

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

 bool FastTriangleIntersect(inout RayPayload Payload, int TriangleIndex)
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
   
   if(Payload.Distance > t)
   {
      Payload.Distance = t;
      Payload.IntersectionPoint = Ray;
      Payload.ClosestTriangleIndex = TriangleIndex;
   }

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

        if (BVH.nodes[CurrentNode].IsLeaf)
        {
            for (int i = 0; i < BVH.nodes[CurrentNode].NumberOfPrims; i++)
                FastTriangleIntersect(Payload, BVH.nodes[CurrentNode].Primitive[i]);
            
        }
        else
        {
            int Child1 = BVH.nodes[CurrentNode].LeftNode;
            int Child2 = BVH.nodes[CurrentNode].RightNode;

            float Distance1 = FLOAT_MAX;
            float Distance2 = FLOAT_MAX;

            if (Child1 != -1)
                Distance1 = IntersectAABB(Payload, BVH.nodes[Child1].BoundingBox);
            

            if (Child2 != -1)
                Distance2 = IntersectAABB(Payload, BVH.nodes[Child2].BoundingBox);
            

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

uint PCGHash(uint seed)
{
	uint state = seed * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float GenFloat(float Min, float Max)
{
    CurrentSeed = PCGHash(CurrentSeed);
    return Min + ((float(CurrentSeed) / float(UINT_MAX)) * (Max-Min));
}

vec3 GenInUnitSphere()
{
    return vec3(GenFloat(-1.0, 1.0), GenFloat(-1.0, 1.0), GenFloat(-1.0, 1.0));
}

bool InBounds(vec3 Value, vec3 MinBounds, vec3 MaxBounds)
{
    return  (Value.x >= MinBounds.x && Value.y >= MinBounds.y && Value.z >= MinBounds.z) &&
            (Value.x <= MaxBounds.x && Value.y <= MaxBounds.y && Value.z <= MaxBounds.z);
}

//TODO change to get material from triangle itself as this method is very buggy

vec4 StartPath(vec3 WorldPosition, vec3 Normal, vec3 InitialColor, vec3 InitalEmission)
{
    vec3 Throughput = vec3(1.0);
    Throughput *= InitialColor;
    Throughput += InitalEmission;


    RayPayload Payload;
    Payload.Origin = WorldPosition + Normal * 0.005;
    Payload.Direction = normalize(GenInUnitSphere() + Normal);
    Payload.Distance = FLOAT_MAX;
    Payload.ClosestTriangleIndex = -1;

    vec2 Min = vec2(0.0, 0.0);
    vec2 Max = vec2(1.0, 1.0);

    for(int i = 0; i < MAX_BOUNCES; i++)
    {
         TraceRay(Payload);

         if(Payload.ClosestTriangleIndex == -1)
            return vec4(Throughput, 1.0);


         vec4 Coord = ViewProjection * vec4(Payload.IntersectionPoint, 1.0);
         
         vec2 ScreenCoord = (Coord.xy + 1.0) * 0.5;

       
         if(ScreenCoord.x > Max.x || ScreenCoord.y > Max.y || ScreenCoord.x < Min.x || ScreenCoord.y < Min.y)
            return vec4(Throughput, 1.0);
         
         ivec2 Coordinates = ivec2(ScreenCoord * imageSize(EmissionAndRoughnessBuffer));

         Throughput *= imageLoad(ColorBuffer, Coordinates).rgb;
         Throughput += imageLoad(EmissionAndRoughnessBuffer, Coordinates).rgb;

         float Roughness = imageLoad(EmissionAndRoughnessBuffer, Coordinates).a;

         Payload.Origin = Payload.IntersectionPoint + Payload.Normal * 0.005;
         Payload.Direction = normalize(GenInUnitSphere() * Roughness + Payload.Normal);
    }


    return vec4(Throughput, 1.0);
}


void main()
{
    ivec2 Coordinate = ivec2(gl_GlobalInvocationID.xy);

    CurrentSeed = FrameIndex;

    float DepthValue = texelFetch(DepthBuffer, Coordinate, 0).r;

    if(DepthValue == 1.0)
        return;
    
    vec2 NormalizedCoord = vec2(Coordinate) / imageSize(ColorBuffer);

    vec4 WorldPosition = InverseView * InverseProjection * vec4(NormalizedCoord * 2.0 - 1.0,
                                                                DepthValue * 2.0 - 1.0, 1.0);

    WorldPosition /= WorldPosition.w;    
    

    vec3 InitialColor = imageLoad(ColorBuffer, Coordinate).rgb;
    vec3 InitialEmission = imageLoad(EmissionAndRoughnessBuffer, Coordinate).rgb;
    vec3 Normal = imageLoad(NormalBuffer, Coordinate).rgb;

    vec4 EndColor = StartPath(WorldPosition.xyz, Normal, InitialColor, InitialEmission);

    if(FrameIndex == 1)
        imageStore(AccumulationBuffer, Coordinate, vec4(0.0, 0.0, 0.0, 0.0));
    
    vec4 AvgColor = imageLoad(AccumulationBuffer, Coordinate) + EndColor;

    imageStore(AccumulationBuffer, Coordinate, AvgColor);

    AvgColor /= FrameIndex;
    
    imageStore(ColorBuffer, Coordinate, AvgColor);
}
