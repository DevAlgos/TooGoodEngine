#version 460 core

layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D ColorBuffer;
layout (binding = 1) uniform sampler2D DepthBuffer;

layout(rgba8, binding = 2) writeonly uniform image2D ShadowBuffer;


#define EPSILON 1.192092896e-07F
#define FLOAT_MAX 3.402823466e+38F

#define MAX_STACK_SIZE 1000

uniform mat4 InverseProjection;
uniform mat4 InverseView;

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
    int Primitive[36];
};

readonly layout(std430, binding = 3) buffer TriangleData
{
    Triangle data[];

} Triangles;

readonly layout(std430, binding = 4) buffer AccelerationStructure
{
	BVHNode nodes[];

} BVHStructure;

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
    float Distance = FLOAT_MAX;
    float tmin = (box.MinValue.x - Payload.Origin.x) / Payload.Direction.x; 
    float tmax = (box.MaxValue.x - Payload.Origin.x) / Payload.Direction.x; 

    if (tmin > tmax) 
    {
        float temp = tmin;
        tmin = tmax;
        tmax = temp;
    }

    float tymin = (box.MinValue.y - Payload.Origin.y) / Payload.Direction.y; 
    float tymax = (box.MaxValue.y - Payload.Origin.y) / Payload.Direction.y; 

    if(tymin > tymax)
    {
        float temp = tymin;
        tymin = tymax;
        tymax = temp;
    }

    if ((tmin > tymax) || (tymin > tmax)) 
        return Distance; 
 
    if (tymin > tmin) 
        tmin = tymin; 
 
    if (tymax < tmax) 
        tmax = tymax; 

    float tzmin = (box.MinValue.z - Payload.Origin.z) / Payload.Direction.z; 
    float tzmax = (box.MaxValue.z - Payload.Origin.z) / Payload.Direction.z; 

    if(tzmin > tzmax)
    {
        float temp = tzmin;
        tzmin = tzmax;
        tzmax = temp;
    }

    if ((tmin > tzmax) || (tzmin > tmax)) 
        return Distance; 
 
    if (tzmin > tmin) 
        tmin = tzmin; 
 
    if (tzmax < tmax) 
        tmax = tzmax; 
 
    Distance = tmin;
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
   
    if (abs(a) < EPSILON)
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

    vec2 NormalizedCoord = (vec2(Coordinate) + vec2(0.5)) / ImageSize;
    float DepthValue = texture(DepthBuffer, NormalizedCoord).r;

    if(DepthValue >= 1.0)
        return Payload;
        
    vec4 ViewSpace = InverseProjection * vec4(NormalizedCoord * 2.0 - 1.0, 
                                              DepthValue * 2.0 - 1.0, 1.0);
    

    ViewSpace /= ViewSpace.w;

    vec4 WorldSpace = InverseView * ViewSpace;

	Payload.Origin = WorldSpace.xyz;

    Payload.Direction = normalize(-LightDirection);

    Payload.Origin += Payload.Direction * 0.2;

	TraceRay(Payload);

	return Payload;
}

void main()
{
	ivec2 Coordinate = ivec2(gl_GlobalInvocationID.xy);

	vec3 LightDirection = vec3(0.0, -1.0, 0.0);

	RayPayload Payload = DispatchShadowRay(Coordinate, LightDirection);

    float ShadowVal = 0.0;

    if(!Payload.InLight)
        ShadowVal = 0.5;
    
     imageStore(ShadowBuffer, Coordinate, vec4(ShadowVal));

}

