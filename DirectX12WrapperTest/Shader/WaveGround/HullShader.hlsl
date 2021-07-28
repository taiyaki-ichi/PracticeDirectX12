#include"Header.hlsli"

float CalcTessFactor(float4 v)
{
	float dist = length(mul(world, v).xyz - float3(0, 0, 0));

	//C++‘¤‚©‚çİ’è‚Å‚«‚é‚æ‚¤‚É‚µ‚Ä‚ ‚é
	float tessNear = tessRange.x;
	float tessFar = tessRange.y;

	const float MAX_TESS_FACTOR = 64.f;

	float val = (MAX_TESS_FACTOR - 1) * (dist - tessNear) / (tessFar - tessNear);
	val = max(1, min(MAX_TESS_FACTOR, val));
	return val;
}



HSParameters CalcHSPatchConstants(InputPatch<HSInput, 4> patch)
{
	HSParameters output;

	float4 v[4];
	float3 n[4];

	int indices[][2] = {
		{2,0},{0,1},{1,3},{2,3}
	};

	//–@ü‚ÌŒvZ
	for (int i = 0; i < 4; i++)
	{
		int idx0 = indices[i][0];
		int idx1 = indices[i][1];

		v[i] = 0.5 * (patch[idx0].pos + patch[idx1].pos);
		float uv = 0.5 * (patch[idx0].uv + patch[idx1].uv);
		n[i] = float3(0, 1, 0);
		n[i] = normalize(n[i] - 0.5);
	}

	output.tessFactor[0] = CalcTessFactor(v[0]);
	output.tessFactor[2] = CalcTessFactor(v[2]);
	output.insideTessFactor[0] = 0.5 * (output.tessFactor[0] + output.tessFactor[2]);

	output.tessFactor[1] = CalcTessFactor(v[1]);
	output.tessFactor[3] = CalcTessFactor(v[3]);
	output.insideTessFactor[1] = 0.5 * (output.tessFactor[1] + output.tessFactor[3]);

	return output;
}

[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("CalcHSPatchConstants")]
DSInput main(InputPatch<HSInput, 4> patch, uint i : SV_OutputControlPointID)
{
	DSInput output;
	output.pos = patch[i].pos;
	output.uv = patch[i].uv;
	return output;
}
