#include "structs.hlsli"
#include "LightHeader.hlsli"

Texture2D SurfaceTexture	: register(t0); // "t" registers for textures
Texture2D SpecularTexture	: register(t1); // same
Texture2D NormalTexture		: register(t2);
SamplerState BasicSampler	: register(s0);	// "s" registers for samplers
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
	float4 surfaceColor = float4(SurfaceTexture.Sample(BasicSampler, input.uv).rgb, 1.0f);
	float surfaceSpecular = SpecularTexture.Sample(BasicSampler, input.uv).r;
	float3 surfaceNormalUnpacked = NormalTexture.Sample(BasicSampler, input.uv).rgb * 2 - 1;

	// Feel free to adjust/simplify this code to fit with your existing shader(s)
	// Simplifications include not re-normalizing the same vector more than once!
	float3 N = normalize(input.normal); // Must be normalized here or before
	float3 T = normalize(input.tangent); // Must be normalized here or before
	T = normalize(T - N * dot(T, N)); // Gram-Schmidt assumes T&N are normalized!
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	// Assumes that input.normal is the normal later in the shader
	float3 normal = mul(surfaceNormalUnpacked, TBN); // Note multiplication order!

	// Uncomment to ignore normal maps for testing
	// normal = normalize(input.normal);

	// Call the lighting math functions
	float3 view = normalize(cameraPosition - input.worldPosition);
	float specExponent = SpecExponent(surfaceSpecular);
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
	return float4(returnedLight, 1.0f);
}