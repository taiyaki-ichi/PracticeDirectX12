#include"Header003.hlsli"

[maxvertexcount(3)]
void main(
	triangle float4 input[3] : SV_POSITION, 
	inout TriangleStream< GSOutput > output
)
{
	float3 e1 = normalize(input[1] - input[0]).xyz;
	float3 e2 = normalize(input[2] - input[0]).xyz;

	float3 normal = normalize(cross(e1, e2));

	for (uint i = 0; i < 3; i++)
	{
		GSOutput element;
		element.pos = input[i];
		element.normal = normal;
		output.Append(element);
	}

	output.RestartStrip();
}