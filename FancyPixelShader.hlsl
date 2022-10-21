#include "structs.hlsli"

cbuffer externalData : register(b0) // b0 means the first buffer register
{
	float4 colorTint;
	float totalTime;
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
	float noise = frac(sin(dot(input.uv.y, float2(12.9898,78.233))) * 43758.5453123);

	// WE NEED TO MAKE THIS FANCY
	return float4(1,1,0.5,1) * colorTint * (sin(totalTime) + 2 + noise);
}