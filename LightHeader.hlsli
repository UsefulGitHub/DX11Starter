#ifndef __GGP_LIGHT_HEADER__
#define __GGP_LIGHT_HEADER__

#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2

#define MAX_SPECULAR_EXPONENT 256.0f

struct Light
{
	int Type;
	float3 Direction;
	float Range;
	float3 Position;
	float Intensity;
	float3 Color;
	float SpotFalloff;
	float3 Padding;
};

float3 NormDirToLight(Light light)
{
	return normalize(-light.Direction);
}

float3 Diffuse(float3 normal, float3 dirToLight)
{
	return saturate(dot(normal, dirToLight));
}

float3 Specular(float3 reflection, float3 view, float specExponent)
{
	if (specExponent > 0.05)
	{
		return pow(saturate(dot(reflection, view)), specExponent);
	}
	return float3(0.0f, 0.0f, 0.0f);
}

float SpecExponent(float roughness)
{
	return (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
}

float3 AddDirectionalLight(Light light, float3 normal, float4 colorTint, float3 reflection, float3 view, float specExponent)
{
	return Specular(reflection, view, specExponent) + Diffuse(normal, NormDirToLight(light)) * light.Color * (float3)colorTint;
}

#endif