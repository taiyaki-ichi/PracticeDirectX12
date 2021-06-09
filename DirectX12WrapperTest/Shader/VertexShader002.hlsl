#include"Header002.hlsli"

PSInput main(VSInput input)
{
	PSInput output;
	output.pos = input.pos;
	output.uv = input.uv;
	return output;
}