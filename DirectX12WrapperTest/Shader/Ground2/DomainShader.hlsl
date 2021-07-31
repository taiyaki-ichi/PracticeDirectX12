#include"Header.hlsli"

[domain("quad")]
PSInput main(
	HSParameters input,
	float2 domain : SV_DomainLocation,
	const OutputPatch<DSInput, 4> patch)
{
	PSInput output;

	float3 p0 = lerp(patch[0].pos, patch[1].pos, domain.x).xyz;
	float3 p1 = lerp(patch[2].pos, patch[3].pos, domain.x).xyz;
	float3 pos = lerp(p0, p1, domain.y);

	float2 c0 = lerp(patch[0].uv, patch[1].uv, domain.x);
	float2 c1 = lerp(patch[2].uv, patch[3].uv, domain.x);
	float2 uv = lerp(c0, c1, domain.y);


	pos.y = texHeightMap.SampleLevel(smp, uv, 0).x;
	float4 p = mul(world, float4(pos.xyz, 1));

	float4 n = float4(texNormalMap.SampleLevel(smp, uv, 0).xyz, 0.f);
	output.normal = mul(world, n);

	output.uv = uv;
	output.pos = mul(mul(proj, view), p);

	output.ray = normalize(p.xyz - eye);

	return output;
}
