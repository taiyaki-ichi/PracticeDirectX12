#include"peraHeader.hlsli"

float4 Get5x5GaussianBlur(Texture2D<float4> tex, SamplerState smp, float2 uv, float dx, float dy, float4 rect) {
	float4 ret = tex.Sample(smp, uv);

	float l1 = -dx, l2 = -2 * dx;
	float r1 = dx, r2 = 2 * dx;
	float u1 = -dy, u2 = -2 * dy;
	float d1 = dy, d2 = 2 * dy;
	l1 = max(uv.x + l1, rect.x) - uv.x;
	l2 = max(uv.x + l2, rect.x) - uv.x;
	r1 = min(uv.x + r1, rect.z - dx) - uv.x;
	r2 = min(uv.x + r2, rect.z - dx) - uv.x;

	u1 = max(uv.y + u1, rect.y) - uv.y;
	u2 = max(uv.y + u2, rect.y) - uv.y;
	d1 = min(uv.y + d1, rect.w - dy) - uv.y;
	d2 = min(uv.y + d2, rect.w - dy) - uv.y;


	return float4((
		tex.Sample(smp, uv + float2(l2, u2)).rgb
		+ tex.Sample(smp, uv + float2(l1, u2)).rgb * 4
		+ tex.Sample(smp, uv + float2(0, u2)).rgb * 6
		+ tex.Sample(smp, uv + float2(r1, u2)).rgb * 4
		+ tex.Sample(smp, uv + float2(r2, u2)).rgb

		+ tex.Sample(smp, uv + float2(l2, u1)).rgb * 4
		+ tex.Sample(smp, uv + float2(l1, u1)).rgb * 16
		+ tex.Sample(smp, uv + float2(0, u1)).rgb * 24
		+ tex.Sample(smp, uv + float2(r1, u1)).rgb * 16
		+ tex.Sample(smp, uv + float2(r2, u1)).rgb * 4

		+ tex.Sample(smp, uv + float2(l2, 0)).rgb * 6
		+ tex.Sample(smp, uv + float2(l1, 0)).rgb * 24
		+ ret.rgb * 36
		+ tex.Sample(smp, uv + float2(r1, 0)).rgb * 24
		+ tex.Sample(smp, uv + float2(r2, 0)).rgb * 6

		+ tex.Sample(smp, uv + float2(l2, d1)).rgb * 4
		+ tex.Sample(smp, uv + float2(l1, d1)).rgb * 16
		+ tex.Sample(smp, uv + float2(0, d1)).rgb * 24
		+ tex.Sample(smp, uv + float2(r1, d1)).rgb * 16
		+ tex.Sample(smp, uv + float2(r2, d1)).rgb * 4

		+ tex.Sample(smp, uv + float2(l2, d2)).rgb
		+ tex.Sample(smp, uv + float2(l1, d2)).rgb * 4
		+ tex.Sample(smp, uv + float2(0, d2)).rgb * 6
		+ tex.Sample(smp, uv + float2(r1, d2)).rgb * 4
		+ tex.Sample(smp, uv + float2(r2, d2)).rgb
		) / 256.0f, ret.a);

}


float4 main(Output input) : SV_TARGET
{

	if (input.uv.x < 0.3 && input.uv.y < 0.3)
	{
		float s = texSSAO.Sample(smp, input.uv / 0.3);
		return float4(s, s, s, 1.f);

	}

/*

	float w, h, miplevels;
	tex.GetDimensions(0, w, h, miplevels);
	float dx = 1.0 / w;
	float dy = 1.0 / h;

	float depthDiff = abs(depthTex.Sample(smp, float2(0.5, 0.5)) - depthTex.Sample(smp, input.uv));
	depthDiff = pow(depthDiff, 0.5f);
	float2 uvSize = float2(1, 0.5);
	float2 uvOfst = float2(0, 0);

	//numが2より大きくなると青がかかってしまう。
	//なぜだろう
	int num = 2;

	float t = depthDiff * num;
	float no;
	t = modf(t, no);
	float4 retColor[2];
	retColor[0] = tex.Sample(smp, input.uv);//通常テクスチャ
	if (no == 0.0f) {
		retColor[1] = Get5x5GaussianBlur(texShrink, smp, input.uv * uvSize + uvOfst, dx, dy, float4(uvOfst, uvOfst + uvSize));
	}
	else {
		for (int i = 1; i <= num; ++i) {
			if (i - no < 0)continue;
			retColor[i - no] = Get5x5GaussianBlur(texShrink, smp, input.uv * uvSize + uvOfst, dx, dy, float4(uvOfst, uvOfst + uvSize));
			uvOfst.y += uvSize.y;
			uvSize *= 0.5f;
			if (i - no > 1) {
				break;
			}
		}
	}
	//return lerp(retColor[0], retColor[1], t);

	*/
	
	float4 col = tex.Sample(smp, input.uv);
	//float4 col = lerp(retColor[0], retColor[1], t);

	return float4(col.rgb * texSSAO.Sample(smp, input.uv), col.a);
}

float random(float2 uv) {
	return frac(sin(dot(uv, float2(12.9898f, 78.233f))) * 43758.5453f);
}


//ぼかし用
BlurOutput BlurPS(Output input) 
{
	float w,h,miplevels;
	tex.GetDimensions(0, w, h, miplevels);
	float dx = 1.0 / w;
	float dy = 1.0 / h;
	BlurOutput ret;
	ret.highLum = Get5x5GaussianBlur(texHighLum, smp, input.uv, dx, dy, float4(0, 0, 1, 1));
	ret.col = tex.Sample(smp, input.uv);

	return ret;
}

//SSAO用
float SsaoPS(Output input) : SV_TARGET
{


	float dp = depthTex.Sample(smp, input.uv);//現在のUVの深度

	float w, h, miplevels;
	depthTex.GetDimensions(0, w, h, miplevels);
	float dx = 1.0f / w;
	float dy = 1.0f / h;

	//元の座標を復元する
	float4 respos = mul(invProj, float4(input.uv * float2(2, -2) + float2(-1, 1), dp, 1));
	respos.xyz = respos.xyz / respos.w;
	float div = 0.0f;
	float ao = 0.0f;
	float3 norm = normalize((texNormal.Sample(smp, input.uv).xyz * 2) - 1);
	const int trycnt = 32;
	const float radius = 0.5f;

	if (dp < 1.0f) {
		for (int i = 0; i < trycnt; ++i) {

			float rnd1 = random(float2(i * dx, i * dy)) * 2 - 1;
			float rnd2 = random(float2(rnd1, i * dy)) * 2 - 1;
			float rnd3 = random(float2(rnd2, rnd1)) * 2 - 1;
			float3 omega = normalize(float3(rnd1, rnd2, rnd3));

			omega = normalize(omega);
			float dt = dot(norm, omega);
			float sgn = sign(dt);
			omega *= sgn;
			float4 rpos = mul(proj,float4(respos.xyz + omega * radius, 1));
			rpos.xyz /= rpos.w;
			ao += step(depthTex.Sample(smp, (rpos.xy + float2(1, -1)) * float2(0.5, -0.5)), rpos.z) * dt * sgn;
		}
		ao /= (float)trycnt;
	}
	return 1.0f - ao;

}
