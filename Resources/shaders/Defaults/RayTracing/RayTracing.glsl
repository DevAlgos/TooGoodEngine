#version 460 core

layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D screen;
layout(rgba32f, binding = 1) uniform image2D screenPixelData;

struct Circle
{
	vec4 Position;
	vec4 Radius;
	
	vec4 Albedo;
	vec4 Reflectivity;
	vec4 Roughness;
	vec4 Emission;
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

uniform int FrameIndex;
uniform int NumberOfObjects;
//uniform int SampleRate;
uniform mat4 InverseView;
uniform mat4 InverseProjection;
uniform vec3 CameraPosition;

uniform float ImageWidth;
uniform float ImageHeight;

const float Pi = 3.14159265359;

const uint UINT_MAX = 0xffffffffu;
shared uint CurrentSeed;


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


/*GGX Normal Distribution function

This function is responsible for determining the alignement
of the microfacets to the hafway vector, this is influenced by the 
roughness

*/
float GGXNormalDistribution(float Roughness, float NdotH)
{
	float R2 = pow(Roughness, 2.0);
	float denominator = Pi * (pow(NdotH, 2.0) * (R2 - 1.0) + 1.0);

	return R2 / max(denominator,0.00001);
}

/*Schlick-Checkman Gemoetry shadowing function

This Function describes the self shadowing properties of a microfacet
when a surface is rough the microfacets can overshadow one another
reducing the light the surface reflects.

*/
float G1(float Roughness, float NdotX)
{
	float k = pow(Roughness+1.0,2.0)/8;
	float denominator = NdotX * (1.0 - k) + k;

	return NdotX / max(denominator, 0.00001);
}

// Smith Model
float G(float Roughness, float NdotV, float NdotL)
{
	return G1(Roughness, NdotV) * G1(Roughness, NdotL);
}

/*
 Fresnel-Shlick Function

 this function dictates the amount of reflection that takes place,
 a material has a base reflectivity. The lower the viewing angle between
 the material and the camera the more reflection there is going to take place
*/

vec3 F(vec3 BaseReflectivity, float NdotH)
{
	return BaseReflectivity + (1.0 - BaseReflectivity) * pow(1 - NdotH,5.0);
}

vec3 GenCosineWeightedSample()
{
	float u1 = GenFloat(-1.0, 1.0);
	float u2 = GenFloat(-1.0, 1.0);

	float Angle1 = sqrt(u1);
	float Angle2 = 2 * Pi * u2;

	return vec3(sin(Angle1) * cos(Angle2), cos(Angle2), cos(Angle1) * sin(Angle2));
}

vec3 UniformSampleHemisphere()
{
    // cos(theta) = r1 = y
    // cos^2(theta) + sin^2(theta) = 1 -> sin(theta) = srtf(1 - cos^2(theta))
	float r1 = GenFloat(0.0, 1.0);
	float r2 = GenFloat(0.0, 1.0);

    float sinTheta = sqrt(1 - r1 * r1);
    float phi = 2 * Pi * r2;
    float x = sinTheta * cos(phi);
    float z = sinTheta * sin(phi);
    return vec3(x, r1, z);
}


vec3 GenInUnitSphere()
{
	return vec3(GenFloat(-1.0, 1.0), GenFloat(-1.0, 1.0), GenFloat(-1.0, 1.0));
}



vec4 Hit(in RayPayload payload, in vec3 AccumulatedLight, in vec3 Throughput)
{
	vec3 LightDirection = payload.Direction;
	float Roughness = CircleData.Data[payload.ClosestCircleIndex].Roughness.x;
	
	vec3 ViewDir = normalize(CameraPosition - vec3(CircleData.Data[payload.ClosestCircleIndex].Position));
	vec3 HalfWayDir = normalize(ViewDir + LightDirection);

	float NdotH = max(dot(payload.Normal, HalfWayDir), 0.0);
	float NdotV = max(dot(payload.Normal, ViewDir),0.0);
	float NdotL = max(dot(payload.Normal, LightDirection),0.0);
	float VdotH = max(dot(ViewDir, HalfWayDir), 0.0);

	vec3  F0 = mix(vec3 (0.04), pow(Throughput, vec3 (2.2)), CircleData.Data[payload.ClosestCircleIndex].Reflectivity.x);
	vec3 Specular = F(F0, VdotH); 
	vec3 Diffuse = vec3(1.0) - Specular;

	/*
		The Lambertian model calculates the diffuse contribution
	*/
	vec3 Lambert = (Throughput * NdotL) / Pi;

	/*
		The cook torrance model is responsible for calculating the specular contribution
	*/

	vec3  CookTorranceNumerator = GGXNormalDistribution(Roughness, NdotH) * G(Roughness, NdotV, NdotL) * Specular; 
	float CookTorranceDenom = 4.0 * NdotV * NdotL;

	vec3 CookTorrance = CookTorranceNumerator / max(CookTorranceDenom,0.00001);

	vec3 BRDF = Diffuse * Lambert + CookTorrance;
	vec3 OutgoingLight = BRDF * AccumulatedLight * NdotL;

	return vec4(OutgoingLight,1.0);
}

bool TraceCircleRay(inout RayPayload Payload)
{
	bool Hit = false;

	float a = dot(Payload.Direction, Payload.Direction);

	for (int i = 0; i < NumberOfObjects; i++)
	{
		vec3 CircleCenter =  vec3(CircleData.Data[i].Position);
		vec3 OriginToCenter = Payload.Origin - CircleCenter;

		float b = 2.0 * dot(OriginToCenter, Payload.Direction);
		float c = dot(OriginToCenter, OriginToCenter) - CircleData.Data[i].Radius.x * CircleData.Data[i].Radius.x;

		float discriminant = b * b - 4 * a * c;

		if (discriminant >= 0)
		{
			float t1 = (-b - sqrt(discriminant)) / (2.0 * a);

			if (Payload.ClosestTarget > t1 && t1 > 0.0f)
			{
				Payload.ClosestTarget = t1;
				Payload.ClosestCircleIndex = i;
				Hit = true;
			}

		}
	}

	return Hit;
}

vec4 GenCircleRay(ivec2 Coordinate, float AspectRatio)
{
//	vec4 AverageColor = vec4(0.0);
//
//	for(int i = 0; i < SampleRate; i++)
//	{
//		RayPayload Payload;
//		Payload.ClosestTarget = 3.402823466e+38;
//		Payload.ClosestCircleIndex = 0;
//
//		vec2 NewCoord = vec2(Coordinate.x / 1920.0, Coordinate.y / 1280.0) * 2.0 - 1.0;
//		vec4 Target = InverseProjection * vec4(NewCoord, 1.0, 1.0);
//
//		Payload.Origin = CameraPosition;
//		Payload.Direction =  vec3(InverseView * vec4(normalize(vec3(Target) / Target.w), 0));
//		Payload.Origin *= AspectRatio;
//
//		bool HitOnce = false;
//
//		vec3 AccumulatedLight = vec3(0.0);
//		vec3 Throughput = vec3(1.0);
//		
//		for (int bounce = 0; bounce < 5; bounce++)
//		{
//			bool RayHit = TraceCircleRay(Payload);
//
//			if (RayHit)
//			{
//				Throughput  *= vec3(CircleData.Data[Payload.ClosestCircleIndex].Albedo);
//				AccumulatedLight += vec3(CircleData.Data[Payload.ClosestCircleIndex].Emission);
//
//				Payload.IntersectionPoint = Payload.Origin + Payload.ClosestTarget * Payload.Direction;
//				Payload.Normal = normalize(Payload.IntersectionPoint - vec3(CircleData.Data[Payload.ClosestCircleIndex].Position));
//
//				float Roughness = CircleData.Data[Payload.ClosestCircleIndex].Roughness.x;
//
//				Payload.Origin = Payload.IntersectionPoint + Payload.Normal * 0.0001;
//				Payload.Direction = (UniformSampleHemisphere()*Roughness) + Payload.Normal;
//				
//				HitOnce = true;
//			}
//			else
//			{
//				AccumulatedLight += vec3(0.1, 0.1, 0.1) * Throughput;
//				break;
//			}
//		}
//
//		if(HitOnce)
//			AverageColor += Hit(Payload, AccumulatedLight, Throughput);
//		else
//			AverageColor += vec4(AccumulatedLight, 1.0);
//		
//
//	}

	RayPayload Payload;
	Payload.ClosestTarget = 3.402823466e+38;
	Payload.ClosestCircleIndex = 0;

	vec2 NewCoord = vec2(Coordinate.x / 1920.0, Coordinate.y / 1080.0) * 2.0 - 1.0;
	vec4 Target = InverseProjection * vec4(NewCoord, 1.0, 1.0);
	 
	Payload.Origin = CameraPosition;
	Payload.Direction =  vec3(InverseView * vec4(normalize(vec3(Target) / Target.w), 0));
	Payload.Origin *= AspectRatio;

	bool HitOnce = false;

	vec3 AccumulatedLight = vec3(0.0);
	vec3 Throughput = vec3(1.0);
	
	for (int bounce = 0; bounce < 5; bounce++)
	{
		CurrentSeed += bounce;
		bool RayHit = TraceCircleRay(Payload);

		if (RayHit)
		{
			Throughput  *= vec3(CircleData.Data[Payload.ClosestCircleIndex].Albedo);
			AccumulatedLight += vec3(CircleData.Data[Payload.ClosestCircleIndex].Emission);

			Payload.IntersectionPoint = Payload.Origin + Payload.ClosestTarget * Payload.Direction;
			Payload.Normal = normalize(Payload.IntersectionPoint - vec3(CircleData.Data[Payload.ClosestCircleIndex].Position));

			float Roughness = CircleData.Data[Payload.ClosestCircleIndex].Roughness.x;

			Payload.Origin = Payload.IntersectionPoint + Payload.Normal * 0.0001;
			//Payload.Direction = reflect(Payload.Direction, Payload.Normal);
			//Payload.Direction = normalize((GenInUnitSphere()*Roughness) + Payload.Normal);
			Payload.Direction = (UniformSampleHemisphere()*Roughness) + Payload.Normal;
			HitOnce = true;
		}
		else
		{
			AccumulatedLight += vec3(0.1, 0.1, 0.1) * Throughput;
			break;
		}
	}

	vec4 val = vec4(0.0);

	if(HitOnce)
		val = Hit(Payload, AccumulatedLight, Throughput);
	else
		val = vec4(AccumulatedLight, 1.0);


	vec4 PreviousValue = imageLoad(screenPixelData, Coordinate);
	imageStore(screenPixelData, Coordinate, PreviousValue + val);

	vec4 AverageColor = (PreviousValue + val) / FrameIndex;
	
	return AverageColor;
}



void main()
{
	vec4 value = vec4(0.0);
    ivec2 Coordinate = ivec2(gl_GlobalInvocationID.xy);

	if(FrameIndex == 1)
		imageStore(screenPixelData, Coordinate, vec4(0.0, 0.0, 0.0, 1.0));

	CurrentSeed =  uint(CurrentSeed + length(Coordinate)) * FrameIndex;

	float AspectRatio = ImageWidth / ImageHeight;

	value = GenCircleRay(Coordinate, AspectRatio);

    imageStore(screen, Coordinate, value);
}
