#include"Header003.hlsli"


[maxvertexcount(2)]
void main(
	triangle float4 input[3] : SV_POSITION, 
	inout LineStream< DrawNormalGSOutput > output
)
{
	float3 e1 = normalize(input[1] - input[0]).xyz;
	float3 e2 = normalize(input[2] - input[0]).xyz;

	float3 normal = normalize(cross(e1, e2));

	float3 center = (input[0].xyz + input[1].xyz + input[2].xyz) / 3.0;

	float length = 0.01;
	for (uint i = 0; i < 2; i++)
	{
		DrawNormalGSOutput element;
		float4 pos = float4(center, 1.f);
		pos.xyz += normal * length * i;
		element.pos = mul(mul(proj, view), pos);
		output.Append(element);
	}
}