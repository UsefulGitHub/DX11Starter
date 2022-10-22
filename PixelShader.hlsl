#include "structs.hlsli"
#include "LightHeader.hlsli"

cbuffer externalData : register(b0) // b0 means the first buffer register
{
	float4 colorTint;
	float roughness;
	float3 cameraPosition;
	float3 ambientLight;
	Light directionalLight1;
	Light directionalLight2;
	Light directionalLight3;
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
	// Interpolation of normals across the face of a triangle results in non-unit vectors (so we do this)
	float3 normal = normalize(input.normal);

	// Call the lighting math functions
	float3 view = normalize(cameraPosition - input.worldPosition);
	float specExponent = SpecExponent(roughness);
	// Add light toghether
	float3 returnedLight =
		AddDirectionalLight(directionalLight1, normal, colorTint, reflect(directionalLight1.Direction, normal), view, specExponent) +
		AddDirectionalLight(directionalLight2, normal, colorTint, reflect(directionalLight2.Direction, normal), view, specExponent) +
		AddDirectionalLight(directionalLight3, normal, colorTint, reflect(directionalLight3.Direction, normal), view, specExponent) +
		((float3)colorTint * ambientLight);

	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	return float4(returnedLight, 1.0f);
}