#include"Header.hlsli"

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return mul(sceneData.shadowMapViewProj,mul(groundData.world,pos));
}