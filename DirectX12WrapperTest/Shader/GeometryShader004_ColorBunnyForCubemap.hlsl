#include"Header004_ColorBunnyForCubemap.hlsli"


[maxvertexcount(18)]
void main(
	triangle float4 input[3] : SV_POSITION,
	inout TriangleStream< GSOutput > output
)
{
	float3 e1 = normalize(input[1] - input[0]).xyz;
	float3 e2 = normalize(input[2] - input[0]).xyz;

	float3 normal = normalize(cross(e1, e2));

	for (uint f = 0; f < 6; f++)
	{
		GSOutput element;
		element.renderTargetArrayIndex = f;
		matrix mtxVP = mul(cubemapProj, cubemapView[f]);

		for (uint i = 0; i < 3; i++)
		{
			float4 pos = mul(world, input[i]);
			element.pos = mul(mtxVP, pos);
			element.normal = mul(world, normal);
			output.Append(element);
		}
		output.RestartStrip();
	}
}