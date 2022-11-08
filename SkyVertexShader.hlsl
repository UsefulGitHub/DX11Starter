#include "structs.hlsli"

cbuffer ExternalData : register(b0) // b0 means the first buffer register
{
	matrix view;
	matrix projection;
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
SkyVertexToPixel main(VertexShaderInput input)
{
	// Set up output struct
	SkyVertexToPixel output;

	// Create a copy of the view matrix and set the translation of it to all zeroes
	matrix viewNoTrans = view;
	viewNoTrans._14 = 0;
	viewNoTrans._24 = 0;
	viewNoTrans._34 = 0;

	// Apply projection and updated view to the input position
	matrix vp = mul(projection, viewNoTrans);
	output.screenPosition = mul(vp, float4(input.localPosition, 1.0f));

	// Ensure that the output depth will be 1.0 after the shader
	// After automatic persp. divide, which divides w/z, z will be 1.0 if z & w are equal
	output.screenPosition.z = output.screenPosition.w;

	// The sample direction is from 0,0,0 to the vertex's position, so sample dir is an easy answer!
	output.sampleDir = input.localPosition;

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}