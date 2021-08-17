#include"Header.hlsli"

float4 main(float4 pos : POSITION, float4 normal : NORMAL, uint id : SV_InstanceID) : SV_POSITION
{
	return mul(sceneData.shadowMapViewProj,mul(bunnyData.world[id],pos));
}