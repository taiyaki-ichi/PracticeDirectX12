#include"Header.hlsli"

float main(PSInput input) : SV_TARGET
{
	//unsigned?
	return input.pos.y > 0.f ? input.pos.y : 0.f;
}