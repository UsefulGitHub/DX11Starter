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

float3 NormDirToDirLight(Light light)
{
	return normalize(-light.Direction);
}

float3 NormDirToPointLight(Light light, float3 worldPos)
{
	return normalize(worldPos - light.Position);
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

float Attenuate(Light light, float3 worldPos)
{
	float dist = distance(light.Position, worldPos);
	float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
	return att * att;
}

// Call this on each directional light
float3 AddDirLightDiffuse(
	Light light,
	float3 normal,
	float4 colorTint
)
{
	return Diffuse(normal, NormDirToDirLight(light))
		* light.Color
		* (float3)colorTint;
}

// Call this on each directional light
float3 AddDirLightSpecular(
	Light light,
	float4 colorTint,
	float3 reflection,
	float3 view,
	float specExponent
)
{
	return Specular(reflection, view, specExponent)
		* light.Color
		* (float3)colorTint;
}

// Call this on each point light
float3 AddPointLightDiffuse(
	Light light,
	float3 normal,
	float4 colorTint,
	float3 worldPos
)
{
	return Attenuate(light, worldPos) * // Multiply the light calculated below by its attenuation
		(
			// Effectively the directional light equation below here
			Diffuse(normal, NormDirToPointLight(light, worldPos))
			* light.Color
			* (float3)colorTint
		);
}

// Call this on each point light
float3 AddPointLightSpecular(
	Light light,
	float4 colorTint,
	float3 reflection,
	float3 view,
	float3 specExponent,
	float3 worldPos
)
{
	return Attenuate(light, worldPos) * // Multiply the light calculated below by its attenuation
		(
			// Effectively the directional light equation below here
			Specular(reflection, view, specExponent)
			* light.Color
			* (float3)colorTint
		);
}

#endif