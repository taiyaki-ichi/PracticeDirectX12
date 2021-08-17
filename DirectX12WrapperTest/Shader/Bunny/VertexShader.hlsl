#include"Header.hlsli"

PSInput main(float4 pos : POSITION, float4 normal : NORMAL, uint id : SV_InstanceID)
{
	PSInput output;
	pos = mul(bunnyData.world[id], pos);
	output.svpos = mul(mul(sceneData.proj, sceneData.view), pos);
	output.pos = pos;
	normal.w = 0.f;
	output.normal = normalize(mul(bunnyData.world[id], normal));
	output.ray = normalize(pos.xyz - sceneData.eye);
	output.tpos = mul(sceneData.shadowMapViewProj, output.pos);
	return output;
}