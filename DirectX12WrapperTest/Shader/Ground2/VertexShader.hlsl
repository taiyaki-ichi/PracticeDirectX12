#include"HEader.hlsli"

HSInput main(VSInput input)
{
	HSInput output;
	output.pos = input.pos;
	output.uv = input.uv;

	return output;
}