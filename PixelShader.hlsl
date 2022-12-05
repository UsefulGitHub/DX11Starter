#include "structs.hlsli"
#include "LightHeader.hlsli"
#include "TextureFunctions.hlsli"

// The texture set
Texture2D AlbedoMap1			: register(t0); // "t" registers for textures
Texture2D NormalMap1			: register(t1); // same
Texture2D RoughnessMap1			: register(t2);
Texture2D MetalnessMap1			: register(t3);
Texture2D AlbedoMap2			: register(t4);
Texture2D NormalMap2			: register(t5);
Texture2D RoughnessMap2			: register(t6);
Texture2D MetalnessMap2			: register(t7);
Texture2D SplatMap				: register(t8);
Texture2D TerrainMap			: register(t9);
SamplerState BasicSampler		: register(s0);	// "s" registers for samplers

// Big External Data cbuffer
cbuffer externalData		: register(b0) // b0 means the first buffer register
{
	float4 colorTint;
	float3 cameraPosition;
	Light directionalLight1;
	Light directionalLight2;
	Light directionalLight3;
	Light pointLight1;
	Light pointLight2;
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Load texture 1
	
	// Get the color at this pixel's uv position
	float4 surfaceColor1 = SampleAlbedo(AlbedoMap1, BasicSampler, input.uv);

	// Get the normals and the TBN, then correct the normals for this position
	// SampleNormal does the TBN inside :)
	float3 normal1 = SampleNormal(NormalMap1, BasicSampler, input.uv, input.normal, input.tangent);

	// Get the metalness and roughness data
	float roughness1 = SampleMetalnessRoughness(RoughnessMap1, BasicSampler, input.uv);
	float metalness1 = SampleMetalnessRoughness(MetalnessMap1, BasicSampler, input.uv);
	
	// Load texture 2
	float4 surfaceColor2 = SampleAlbedo(AlbedoMap2, BasicSampler, input.uv);

	// Get the normals and the TBN, then correct the normals for this position
	// SampleNormal does the TBN inside :)
	float3 normal2 = SampleNormal(NormalMap2, BasicSampler, input.uv, input.normal, input.tangent);

	// Get the metalness and roughness data
	float roughness2 = SampleMetalnessRoughness(RoughnessMap2, BasicSampler, input.uv);
	float metalness2 = SampleMetalnessRoughness(MetalnessMap2, BasicSampler, input.uv);

	// Load splat map
	float splat = SplatMap.Sample(BasicSampler, input.uv).r;

	// Load terrain map
	float terrain = TerrainMap.Sample(BasicSampler, input.uv).r;

	float4 surfaceColor = lerp(surfaceColor1, surfaceColor2, 1 - splat);
	float3 normal = lerp(normal1, normal2, 1 - splat);
	float roughness = lerp(roughness1, roughness2, 1 - splat);
	float metalness = lerp(metalness1, metalness2, 1 - splat);

	// Get the normalized view for specular highlights
	float3 view = normalize(cameraPosition - input.worldPosition);
	
	// Get the specular color for all the specular lights to use
	float3 specColor = GetSpecularColor(surfaceColor, metalness);

	// Add light toghether
	float3 returnedLight =
		DirectionalLight(directionalLight1, normal, roughness, metalness, specColor, surfaceColor, colorTint, view) +
		DirectionalLight(directionalLight2, normal, roughness, metalness, specColor, surfaceColor, colorTint, view) +
		DirectionalLight(directionalLight3, normal, roughness, metalness, specColor, surfaceColor, colorTint, view) +
		PointLight(pointLight1, normal, roughness, metalness, specColor, surfaceColor, colorTint, view, input.worldPosition) +
		PointLight(pointLight2, normal, roughness, metalness, specColor, surfaceColor, colorTint, view, input.worldPosition);

	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	//return float4(pow(returnedLight, 1.0f / 2.2f), 1.0f);
	return float4(terrain.rrr, 1.0f);
}