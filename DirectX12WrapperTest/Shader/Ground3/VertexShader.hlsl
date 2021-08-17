#include"Header.hlsli"

PSInput main(float4 pos : POSITION)
{
	PSInput output;
	pos = mul(groundData.world, pos);
	output.svpos = mul(mul(sceneData.proj, sceneData.view), pos);
	output.pos = pos;
	output.tpos = mul(sceneData.shadowMapViewProj, pos);
	return output;
}