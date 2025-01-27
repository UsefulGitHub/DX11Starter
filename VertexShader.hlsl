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
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;
	
	// Here we're essentially passing the input position directly through to the next
	// stage (rasterizer), though it needs to be a 4-component vector now.  
	// - To be considered within the bounds of the screen, the X and Y components 
	//   must be between -1 and 1.  
	// - The Z component must be between 0 and 1.  
	// - Each of these components is then automatically divided by the W component, 
	//   which we're leaving at 1.0 for now (this is more useful when dealing with 
	//   a perspective projection matrix, which we'll get to in the future).
	// output.screenPosition = float4(input.localPosition + offset, 1.0f);
	matrix wvp = mul(projection, mul(view, world));
	
	// Here go the output values
	output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));
	output.uv = input.uv; // The uvs are just passing through here
	output.normal = mul((float3x3)worldInvTrans, input.normal);
	output.worldPosition = mul(world, float4(input.localPosition, 1.0f)).xyz;
	output.tangent = mul((float3x3)world, input.tangent);

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}