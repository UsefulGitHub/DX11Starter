#include "structs.hlsli"
#include "LightHeader.hlsli"
#include "TextureFunctions.hlsli"

// The texture set
Texture2D AlbedoMap			: register(t0); // "t" registers for textures
Texture2D NormalMap			: register(t1); // same
Texture2D RoughnessMap		: register(t2);
Texture2D MetalnessMap		: register(t3);
SamplerState BasicSampler	: register(s0);	// "s" registers for samplers

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
	// Get the color at this pixel's uv position
	float4 surfaceColor = SampleAlbedo(AlbedoMap, BasicSampler, input.uv);
	
	// Get the normals and the TBN, then correct the normals for this position
	// SampleNormal does the TBN inside :)
	float3 normal = SampleNormal(NormalMap, BasicSampler, input.uv, input.normal, input.tangent);

	// Get the metalness and roughness data
	float roughness = SampleMetalnessRoughness(RoughnessMap, BasicSampler, input.uv);
	float metalness = SampleMetalnessRoughness(MetalnessMap, BasicSampler, input.uv);

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
	return float4(pow(returnedLight, 1.0f / 2.2f), 1.0f);
}