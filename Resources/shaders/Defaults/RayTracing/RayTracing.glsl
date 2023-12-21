#version 460 core

layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D screen;

struct Circle
{
	vec4 Position;
	vec4 DiffuseColor;
    vec4 Radius;
};

struct RayPayload
{
	vec3 Direction;
	vec3 Origin;

	vec3 Normal;
	vec3 IntersectionPoint;

	float ClosestTarget;
	int ClosestCircleIndex;
};


layout(std430, binding = 1) readonly buffer CircleD
{
    Circle Data[];
} CircleData;



uniform mat4 InverseView;
uniform mat4 InverseProjection;
uniform vec3 PlayerOrigin;



vec4 Hit(in RayPayload payload, in int CircleIndex, in vec3 AccumulatedColor)
{
	vec3 LightDirection = vec3(-1.0, -1.0, 1.0);
	float DiffuseIntensity = max(dot(payload.Normal, -LightDirection), 0.004);		
	vec3 Diffuse = AccumulatedColor * DiffuseIntensity;
	Diffuse = clamp(Diffuse, AccumulatedColor*0.01, vec3(1.0, 1.0, 1.0));

	return vec4(Diffuse,1.0);
}

bool TraceCircleRay(inout RayPayload Payload, inout vec3 AccumulatedColor)
{
	bool Hit = false;

	float a = dot(Payload.Direction, Payload.Direction);

	for (int i = 0; i < 5*4; i++)
	{
		vec3 CircleCenter =  vec3(CircleData.Data[i].Position);
		vec3 OriginToCenter = Payload.Origin - CircleCenter;

		float b = 2.0 * dot(OriginToCenter, Payload.Direction);
		float c = dot(OriginToCenter, OriginToCenter) - CircleData.Data[i].Radius.x * CircleData.Data[i].Radius.x;

		float discriminant = b * b - 4 * a * c;

		if (discriminant >= 0)
		{
			float t1 = (-b + sqrt(discriminant)) / (2.0 * a);

			if (Payload.ClosestTarget >= t1)
			{
				Payload.ClosestTarget = t1;
				Payload.ClosestCircleIndex = i;
				Hit = true;
				AccumulatedColor += vec3(CircleData.Data[i].DiffuseColor) * 0.8;
				AccumulatedColor = max(AccumulatedColor, vec3(1.0, 1.0, 1.0));
			}

		}
	}

	return Hit;
}

vec4 GenCircleRay(ivec2 Coordinate, float AspectRatio)
{
	RayPayload Payload;
	Payload.ClosestTarget = 3.402823466e+38;
	Payload.ClosestCircleIndex = 0;

	vec2 NewCoord = vec2(Coordinate.x / 1920.0, Coordinate.y / 1280.0) * 2.0 - 1.0;
	
	vec4 Target = InverseView * vec4(NewCoord, 1.0, 1.0);
	vec3 RayDir = vec3(InverseProjection * vec4(normalize(vec3(Target)) / Target.w, 0.0));

	Payload.Origin = PlayerOrigin;
	Payload.Direction = RayDir;

	Payload.Origin *= AspectRatio;

	bool HitOnce = false;

	vec3 AccumulatedColor = vec3(0.0);
	
	for (int bounce = 0; bounce < 2; bounce++)
	{
		bool RayHit = TraceCircleRay(Payload, AccumulatedColor);

		if (RayHit)
		{
			Payload.IntersectionPoint = Payload.Origin + Payload.ClosestTarget * Payload.Direction;
			Payload.Normal = Payload.IntersectionPoint - vec3(CircleData.Data[Payload.ClosestCircleIndex].Position);

			Payload.Origin = Payload.IntersectionPoint + normalize(Payload.Normal) * 0.0001;
			Payload.Direction = reflect(Payload.Direction, Payload.Normal);
			HitOnce = true;
		}
		else
		{
			break;
		}
	}

	if(HitOnce)
		return Hit(Payload, Payload.ClosestCircleIndex, AccumulatedColor);
	else
		return vec4(0.0, 0.0, 0.0, 1.0);
}



void main()
{
	vec4 value = vec4(0.0);
    ivec2 Coordinate = ivec2(gl_GlobalInvocationID.xy);

	float AspectRatio = 1920.0 / 1080.0;

	value = GenCircleRay(Coordinate, AspectRatio);

    imageStore(screen, Coordinate, value);
}
