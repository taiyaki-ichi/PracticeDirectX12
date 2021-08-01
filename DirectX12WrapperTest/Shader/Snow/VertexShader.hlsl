#include"Header.hlsli"

float4 main(float4 pos : POSITION) : SV_POSITION
{
	float4 output = pos + move;
	float3 trip = floor(((center.xyz - output.xyz) * rangeR + 1) * 0.5);
	trip *= (range * 2.f);
	output += float4(trip, 1.f);
	return output;
}