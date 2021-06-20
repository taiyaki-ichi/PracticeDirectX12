#include"Header.hlsli"

VSOutput main(float4 pos : POSITION ,float4 normal : NORMAL)
{
	VSOutput output;
	output.pos = pos;
	output.normal = normal;
	return output;
}