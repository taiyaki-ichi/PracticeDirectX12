#include"Header.hlsli"

[maxvertexcount(6)]
void main(
	point float4 input[1] : SV_POSITION,
	inout TriangleStream< PSInput > output
)
{
	const float2 uv[4]={
		{0,0},{1,0},{0,1},{1,1}
	};

	const uint index[6]={
		0,1,3,0,3,2
	};

	for (uint i = 0; i < 6; i++)
	{
		PSInput element;
		element.svpos = input[0];
		//d‚¢?
		element.svpos.x += sin(input[0].x * 0.2) * sin(input[0].y * 0.3) * sin(input[0].z * 0.8) * 2.f;
		element.svpos.z += sin(input[0].x * 0.8) * sin(input[0].y * 0.1) * sin(input[0].z * 0.5) * 2.f;

		element.svpos += float4((uv[index[i]].x - 0.5f) * float3(1, 0, 0), 0) * size;
		element.svpos += float4((uv[index[i]].y - 0.5f) * float3(0, 1, 0), 0) * size;
		element.svpos = mul(proj,element.svpos);
		element.uv = uv[index[i]];
		output.Append(element);

		if (i % 3 == 2)
			output.RestartStrip();
	}
}