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
	float roughness;
	float3 cameraPosition;
	float3 ambientLight;
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
	float3 normal = SampleNormal(NormalMap, BasicSampler, input.uv, input.normal, input.tangent);

	// Get the normalized view for specular lights
	float3 view = normalize(cameraPosition - input.worldPosition);
	
	// Add light toghether
	float3 returnedLight =
		AddDirLightDiffuse(directionalLight1, normal, surfaceColor * colorTint) +
		(AddDirLightSpecular(directionalLight1, surfaceColor * colorTint, reflect(NormDirToDirLight(directionalLight1), normal), view, specExponent)) +
		AddDirLightDiffuse(directionalLight2, normal, surfaceColor * colorTint) +
		(AddDirLightSpecular(directionalLight2, surfaceColor * colorTint, reflect(NormDirToDirLight(directionalLight2), normal), view, specExponent)) +
		AddDirLightDiffuse(directionalLight3, normal, surfaceColor * colorTint) +
		(AddDirLightSpecular(directionalLight3, surfaceColor * colorTint, reflect(NormDirToDirLight(directionalLight3), normal), view, specExponent)) +
		AddPointLightDiffuse(pointLight1, normal, surfaceColor * colorTint, input.worldPosition) +
		(AddPointLightSpecular(pointLight1, surfaceColor * colorTint, reflect(NormDirToPointLight(pointLight1, input.worldPosition), normal), view, specExponent, input.worldPosition)) +
		AddPointLightDiffuse(pointLight2, normal, surfaceColor * colorTint, input.worldPosition) +
		(AddPointLightSpecular(pointLight2, surfaceColor * colorTint, reflect(NormDirToPointLight(pointLight2, input.worldPosition), normal), view, specExponent, input.worldPosition)) +
		((float3)(surfaceColor * colorTint) * ambientLight);

	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	return float4(pow(returnedLight, 1.0f / 2.2f), 1.0f);
}