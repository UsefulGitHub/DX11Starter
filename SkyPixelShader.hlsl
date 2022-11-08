#include "structs.hlsli"

TextureCube SkyCube			: register(t0);
SamplerState SkySampler		: register(s0);

float4 main(SkyVertexToPixel input) : SV_TARGET
{
	float4 skyColor = float4(SkyCube.Sample(SkySampler, input.sampleDir).rgb, 1.0f);
	return skyColor;
}