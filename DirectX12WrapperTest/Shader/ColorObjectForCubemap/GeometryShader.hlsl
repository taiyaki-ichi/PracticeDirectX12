#include"Header.hlsli"

[maxvertexcount(18)]
void main(
	triangle VSOutput input[3] : SV_POSITION,
	inout TriangleStream< GSOutput > output
)
{

	for (uint f = 0; f < 6; f++)
	{
		GSOutput element;
		element.renderTargetArrayIndex = f;
		matrix mtxVP = mul(cubemapProj, cubemapView[f]);

		for (uint i = 0; i < 3; i++)
		{
			float4 pos = mul(world, input[i].pos);
			element.pos = mul(mtxVP, pos);
			element.normal = mul(world, input[i].normal);
			output.Append(element);
		}
		output.RestartStrip();
	}
}