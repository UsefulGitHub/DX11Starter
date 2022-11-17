#ifndef __GGP_LIGHT_HEADER__
#define __GGP_LIGHT_HEADER__

#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2

#define MAX_SPECULAR_EXPONENT 512.0f

// Minimum roughness for when spec distribution function denominator goes to zero
static const float MIN_ROUGHNESS = 0.0000001f; // 6 zeros after decimal

// Handy to have this as a constant
static const float PI = 3.14159265359f;

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
// Lambert diffuse BRDF - Same as the basic lighting diffuse calculation!
// - NOTE: this function assumes the vectors are already NORMALIZED!
float3 Diffuse(float3 normal, float3 dirToLight)
{
	return saturate(dot(normal, dirToLight));
}

// Calculates diffuse amount based on energy conservation
//
// diffuse - Diffuse amount
// specular - Specular color (including light color)
// metalness - Surface metalness amount
//
// Metals should have an albedo of (0, 0, 0) mostly
// See slide 65: http://blog.selfshadow.com/publications/s2014-shading-course/hoffman/s2014_pbs_physics_math_slides.pdf
float3 DiffuseEnergyConserve(float3 diffuse, float3 specular, float metalness)
{
	return diffuse * ((1 - saturate(specular)) * (1 - metalness));
}

// The Microfacet BRDF takes
//	n - the normal after normal mapping
//	l - normalized vector to the light (light vector)
//	v - normalized vector to the camera (view vector)
//	rougness - from the roughness map
//	specColor - calculated from albedo and metalness
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float specColor)
{
	return (0, 0, 0);
}

// GGX (Trowbridge-Reitz)
//
// a - Roughness
// h - Half vector
// n - Normal
//
// D(h,n) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1)^2
float SpecDistribution(float3 n, float3 h, float roughness)
{
	// Precalc
	float NdotH = saturate(dot(n, h));
	float NdotHSq = NdotH * NdotH;
	float a = roughness * roughness;
	float a2 = max(a * a, MIN_ROUGHNESS); // Applied after remap!

	// ((n dot h)^2 * (a^2 - 1) + 1)
	float denomToSquare = NdotH2 * (a2 - 1) + 1;
	// Can go to zero if roughness is 0 and NdotH is 1:
	// MIN_ROUGHNESS helps here

	// Final value
	return a2 / (PI * denomToSquare * denomToSquare);
}

// Fresnel term - Schlick approx.
//
// v - View vector
// h - Half vector
// f0 - Value when 1 = n (full specular color)
// 
// F(v, h, f0) = f0 + (1 - f0)(1 - (v dot h))^5
float3 Fresnel(float3 v, float3 h, float3 f0)
{
	// Precalc
	float VdotH = saturate(dot(v, h));

	// Final value
	return f0 + (1 - f0) * pow(1 - VdotH, 5);
}

// Geometric shadowing - Schlick-GGX (based on Schlick-Beckmann)
// - k is remapped to a / 2
// - roughness is remapped to (r + 1)/2
//
// n - Normal
// v - View vector
//
// G(l, v)
float GeometricShadowing(float3 n, float3 v, float roughness)
{
	// End result of remapping
	float k = pow(roughness + 1, 2) / 8.0f;
	float NdotV = saturate(dot(n, v));

	// Final value
	return NdotV / (NdotV * (1 - k) + k);
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