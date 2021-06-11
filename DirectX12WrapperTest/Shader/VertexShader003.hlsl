#include"Header003.hlsli"

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return mul(mul(proj, view), pos);
}