#ifndef __GGP_SHADER_STRUCTS__
#define __GGP_SHADER_STRUCTS__

// Struct representing the data we're sending down the pipeline
// - Should match our pixel shader's input (hence the name: Vertex to Pixel)
// - At a minimum, we need a piece of data defined tagged as SV_POSITION
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;	// XYZW position (System Value Position)
	float2 uv				: TEXCOORD;		// UV position
	float3 normal			: NORMAL;		// Pixel's surface normal's direction
	float3 worldPosition	: POSITION;		// Pixel's world position
};

#endif