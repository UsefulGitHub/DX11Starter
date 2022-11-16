#include "structs.hlsli"

cbuffer ExternalData : register(b0) // b0 means the first buffer register
{
	// Describing the layout of this buffer
	// - Variable names don't mean much going from CPU to GPU memory,
	// - The GPU just takes in what is bound to the context when we call Draw()
	// - So the thing we need to define is the data types of whatever we will put in register 0
	// - (we have to know first, and it has to be reliable)
	// - The shader will take the first 4 bytes and make a float4, and the next 3 and make a float3
	// - If nothing has been put in b0, it will take all the zeroes in the register and use them
	matrix world;
	matrix view;
	matrix projection;
	matrix worldInvTrans;
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main(VertexShaderInput input)
{
	// Set up output struct
	VertexToPixel output;

	// Calculate the screen position of this pixel
	matrix wvp = mul(projection, mul(view, world));
	output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));
	
	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}