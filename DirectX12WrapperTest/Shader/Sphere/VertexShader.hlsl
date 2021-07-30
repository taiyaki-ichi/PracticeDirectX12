#include"Header.hlsli"

PSInput main(float4 pos : POSITION, float4 normal : NORMAL,uint id : SV_InstanceID)
{
	PSInput output;
	pos = mul(world[id], pos);
	output.svpos = mul(mul(proj,view), pos);
	output.pos = pos;
	normal.w = 0.f;
	output.normal = normalize(mul(world[id], normal));
	output.ray = normalize(pos.xyz - mul(view, eye));
	return output;
}