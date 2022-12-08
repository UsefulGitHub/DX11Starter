#ifndef __GGP_TEXTURE_FUNCTIONS__
#define __GGP_TEXTURE_FUNCTIONS__

// The fresnel value for non-metals (dielectrics)
// Page 9: "F0 of nonmetals is now a constant 0.04"
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
static const float F0_NON_METAL = 0.04f;

float3x3 GetTBN(float3 normal, float3 tangent)
{
	// Feel free to adjust/simplify this code to fit with your existing shader(s)
	// Simplifications include not re-normalizing the same vector more than once
	float3 N = normalize(normal); // Must be normalized here or before
	float3 T = normalize(tangent); // Must be normalized here or before
	T = normalize(T - N * dot(T, N)); // Gram-Schmidt assumes T&N are normalized!
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);
	return TBN;
}

float4 SampleAlbedo(Texture2D map, SamplerState samp, float2 uv)
{
	return float4(pow(map.Sample(samp, uv).rgb, 2.2f), 1.0f);
}

float3 SampleNormal(Texture2D map, SamplerState samp, float2 uv, float3 normal, float3 tangent)
{
	float3 surfaceNormalUnpacked = map.Sample(samp, uv).rgb * 2 - 1;
	float3x3 TBN = GetTBN(normal, tangent);
	return mul(surfaceNormalUnpacked, TBN); // Note multiplication order!
}

float SampleMetalnessRoughness(Texture2D map, SamplerState samp, float2 uv)
{
	return map.Sample(samp, uv).r;
}

float3 GetSpecularColor(float4 albedoColor, float metalness)
{
	// Specular color determination
	// Assume albedo texture is actually holding specular color where metalness is 1
	// 
	// Note the use of lerp here - metal is generally 0 or 1, but might be in between
	// because of linear texture sampling, so we lerp to accept the fuzzy answers
	return lerp(F0_NON_METAL.rrr, albedoColor.rgb, metalness);
}

// We have to convert the normal from cartesian coordinates to spherical coordinates
// The origin can be zero in cartesian
// In the sphere, p will be the same for all points
#endif