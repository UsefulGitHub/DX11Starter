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
	float denomToSquare = NdotHSq * (a2 - 1) + 1;
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

// The Microfacet BRDF takes
//
// 	n - the normal after normal mapping
//	l - normalized vector to the light (light vector)
//	flv - normalized vector to the camera (view vector)
//	rougness - from the roughness map
//	specColor - calculated from albedo and metalness
//
// f(l,v) = D(h)F(v,h)G(l,v,h) / 4(n dot l)(n dot v)
// D is for Spec Distribution
// F is for Fresnel
// G is for Geometric Shadowing
// Parts of the numerator cancel out parts of the denominator (see below)
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float3 specColor)
{
	// Other vectors
	float3 h = normalize(v + l);

	// Grab various functions
	float D = SpecDistribution(n, h, roughness);
	float3 F = Fresnel(v, h, specColor);
	float G = GeometricShadowing(n, v, roughness) * GeometricShadowing(n, l, roughness);

	// Final f0rmula
	// Den0m d0t products partially concelled by G()
	// See page 16: http://blog.selfshadow.com/publications/s2012-shading-course/hoffman/s2012_pbs_physics_math_notes.pdf
	return (D * F * G) / 4 * (max(dot(n, v), dot(n, l)));
}

float Attenuate(Light light, float3 worldPos)
{
	float dist = distance(light.Position, worldPos);
	float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
	return att * att;
}

float3 DirectionalLight(
	Light light,
	float3 normal,
	float roughness,
	float metalness,
	float3 specColor,
	float4 surfaceColor,
	float4 colorTint,
	float3 view
)
{
	// Get the direction to the directional light
	float3 toLight = NormDirToDirLight(light);

	// Calculate the amounts of each type of light
	float3 diff = Diffuse(normal, toLight);
	float3 spec = MicrofacetBRDF(normal, toLight, view, roughness, specColor);

	float3 balancedDiff = DiffuseEnergyConserve(diff, spec, metalness);

	// Combine the final diffuse and specular values for this light
	return (balancedDiff * ((float3)surfaceColor * (float3)colorTint) + spec) * light.Intensity * light.Color;
}

float3 PointLight(
	Light light,
	float3 normal,
	float roughness,
	float metalness,
	float3 specColor,
	float4 surfaceColor,
	float4 colorTint,
	float3 view,
	float3 worldPos
)
{
	// Get the direction to the point light and calculate attenuation
	float3 toLight = NormDirToPointLight(light, worldPos);
	float attenuation = Attenuate(light, worldPos);

	// Calculate the amounts of each type of light
	float3 diff = Diffuse(normal, toLight);
	float3 spec = MicrofacetBRDF(normal, toLight, view, roughness, specColor);

	float3 balancedDiff = DiffuseEnergyConserve(diff, spec, metalness);

	// Combine the final diffuse and specular values for this light
	return ((balancedDiff * ((float3) surfaceColor * (float3)colorTint) + spec) * light.Intensity * light.Color) * attenuation;
}

#endif