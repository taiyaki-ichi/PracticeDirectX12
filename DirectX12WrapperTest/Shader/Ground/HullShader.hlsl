#include"Header.hlsli"

//距離ベースのテッセレーション係数計算
float CalcTessFactor(float4 v)
{
	float dist = length(mul(world, v).xyz - eye);

	//C++側から設定できるようにしてある
	float tessNear = tessRange.x;
	float tessFar = tessRange.y;

	const float MAX_TESS_FACTOR = 64.f;

	float val = (MAX_TESS_FACTOR - 1) * (dist - tessNear) / (tessFar - tessNear);
	val = max(1, min(MAX_TESS_FACTOR, val));
	return  val;
}


//輪郭線になるような場所のテッセレーション係数の補正用
float CalcNormalBias(float3 p, float3 n)
{
	const float NORMAL_THRESHOLD = 0.85f;//約60度

	float3 fromEye = normalize(p - eye);
	float cos2 = dot(n, fromEye);
	cos2 *= cos2;
	float normalFactor = 1.0 - cos2;
	float bias = max(normalFactor - NORMAL_THRESHOLD, 0) / (1.0 - NORMAL_THRESHOLD);
	return bias * clamp(tessRange.z, 0, 64);
}


HSParameters CalcHSPatchConstants(InputPatch<HSInput, 4> patch)
{
	HSParameters output;

	float4 v[4];
	float3 n[4];

	int indices[][2] = {
		{2,0},{0,1},{1,3},{2,3}
	};

	//法線の計算
	for (int i = 0; i < 4; i++)
	{
		int idx0 = indices[i][0];
		int idx1 = indices[i][1];

		v[i] = 0.5 * (patch[idx0].pos + patch[idx1].pos);
		float uv = 0.5 * (patch[idx0].uv + patch[idx1].uv);
		n[i] = texNormalMap.SampleLevel(smp, uv, 0).xyz;
		n[i] = normalize(n[i] - 0.5);
	}

	output.tessFactor[0] = CalcTessFactor(v[0]);
	output.tessFactor[2] = CalcTessFactor(v[2]);
	output.tessFactor[0] += CalcNormalBias(v[0].xyz, n[0].xyz);
	output.tessFactor[2] += CalcNormalBias(v[2].xyz, n[2].xyz);
	output.insideTessFactor[0] = 0.5 * (output.tessFactor[0] + output.tessFactor[2]);

	output.tessFactor[1] = CalcTessFactor(v[1]);
	output.tessFactor[3] = CalcTessFactor(v[3]);
	output.tessFactor[1] += CalcNormalBias(v[1].xyz, n[1].xyz);
	output.tessFactor[3] += CalcNormalBias(v[3].xyz, n[3].xyz);
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
