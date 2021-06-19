#include"Header.hlsli"

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
		element.pos = mul(mul(proj, view), mul(world, input[i]));
		element.normal = mul(world, normal);
		float3 eyeDir = normalize(mul(world, input[i]).xyz - eye);
		element.reflect = reflect(eyeDir, mul(world, normal).xyz);
		output.Append(element);
	}
}