#version 460 core

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D ColorBuffer;
layout(rgba32f, binding = 1) uniform image2D ReflectAndMetallicBuffer;
layout(rgba32f, binding = 2) uniform image2D EmissionAndRoughnessBuffer;
layout(rgba32f, binding = 3) uniform image2D NormalBuffer;

layout(binding = 4) uniform sampler2D DepthBuffer;

layout(rgba8, binding = 5) readonly uniform image2D ShadowBuffer;

uniform vec3 CameraPosition;

uniform mat4 InverseProjection;
uniform mat4 InverseView;

#define Pi 3.14159265359f
#define EPSILON 1.192092896e-07f

/*
Trowbridge-Reitz GGX

Used to statisticaly appromximate the relative surface area of microfacets
aligned with the halfway vector.
*/

float DistributionGGX(float NdotH, float Roughness)
{
	float a      = pow(Roughness, 2);
    float a2     = pow(a, 2);
    float NdotH2 = pow(NdotH, 2);
	
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = Pi * denom * denom;
	
    return a2 / denom; 
}

/*
Geometry Function which is a combination of GGX and Schlick-Beckmann approximation
This decribes the self shadowing properties of a microfacet where one microfacet
may occlude another (produce a shadow onto another).

*/

float GeometryGGX(float NdotX, float Roughness)
{
	float k = pow(Roughness+1.0,2.0)/8;

	float Denom = NdotX * (1.0 - k) + k;
	return NdotX / max(Denom, EPSILON);
}

float GeometryFun(float NdotV, float NdotL, float Roughness)
{
	return GeometryGGX(NdotV, Roughness) * GeometryGGX(NdotL, Roughness);
}

/*
the fresnel equation dictates how much reflection there is based on
the view angle. The actual fresnel equation is complicated but
this is a sufficient approximation for graphics use.
*/

vec3 FresnelApproximation(float VdotH, vec3 BaseReflectivity)
{
	return BaseReflectivity + (1.0 - BaseReflectivity) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);
}

vec4 ShadePixel(in ivec2 Coordinate, 
				in vec2  NormalizedCoord, 
				in vec3  LightDirection,
				in vec3  Radiance,
				in vec3  WorldPosition)
{
	float DepthValue = texture(DepthBuffer, NormalizedCoord).r;
	
	vec3  Normal       = normalize(imageLoad(NormalBuffer, Coordinate).xyz);
	
	vec3  AlbedoValue    = imageLoad(ColorBuffer, Coordinate).xyz;
	
	vec3  Reflectivity   = imageLoad(ReflectAndMetallicBuffer, Coordinate).xyz;
	float Metallic       = imageLoad(ReflectAndMetallicBuffer, Coordinate).w;
	
	vec3  EmissionValue  = imageLoad(EmissionAndRoughnessBuffer, Coordinate).xyz;
	float RoughnessValue = imageLoad(EmissionAndRoughnessBuffer, Coordinate).w;

	float ShadowVal = imageLoad(ShadowBuffer, Coordinate).x;

	LightDirection = -LightDirection;

	vec3 ViewDir    = normalize(CameraPosition - WorldPosition);
	vec3 HalfwayDir = normalize(LightDirection + ViewDir);

	float NdotL = max(dot(Normal, LightDirection), 0.0);
	float NdotV = max(dot(Normal, ViewDir),0.0);
	float NdotH = max(dot(Normal, HalfwayDir),0.0);
	float VdotH = max(dot(ViewDir, HalfwayDir), 0.0);

	vec3 BaseReflectivity = vec3(0.04);
	BaseReflectivity = mix(BaseReflectivity, AlbedoValue, Metallic);

	vec3 LambertionDiffuse = AlbedoValue / Pi;
	LambertionDiffuse *= (1.0 - (ShadowVal));

	vec3 Ks = FresnelApproximation(VdotH, BaseReflectivity);
	vec3 Kd = vec3(1.0) - Ks;
	Kd *= 1.0 - Metallic;

	vec3 CookTorranceNum =  DistributionGGX(NdotH, RoughnessValue)     *
							GeometryFun(NdotV, NdotL, RoughnessValue)  *
							Ks;

	float CookTorranceDenom = max(4.0 * NdotL * NdotV, EPSILON);

	vec3 CookTorrance = CookTorranceNum / CookTorranceDenom;

	vec3 BRDF = Kd * LambertionDiffuse + CookTorrance; 

	vec3 FinalColor = EmissionValue + BRDF * Radiance * NdotL;
	
	return vec4(FinalColor.xyz, 1.0);
}

void main()
{
	ivec2 Coordinate = ivec2(gl_GlobalInvocationID.xy);

	vec3 LightDirection = vec3(0.0, -1.0, -1.0); //TODO Turn into Shader Storage Buffer for multiple Directional Lights
	
	vec2 NormalizedCoord = (vec2(Coordinate)) / imageSize(ColorBuffer);

	float DepthValue = texelFetch(DepthBuffer, Coordinate, 0).r;

	if(DepthValue == 1.0)
		return; //background	

    vec4 ViewSpace = InverseProjection * vec4(NormalizedCoord * 2.0 - 1.0, 
                                              DepthValue * 2.0 - 1.0, 1.0);
    

    ViewSpace /= ViewSpace.w;

    vec4 WorldSpace = InverseView * ViewSpace;

	vec4 Color = ShadePixel(Coordinate, 
							NormalizedCoord, 
							LightDirection, 
							vec3(1.0, 1.0, 1.0) * 3.0,
							WorldSpace.xyz);

	imageStore(ColorBuffer, Coordinate, Color);
}