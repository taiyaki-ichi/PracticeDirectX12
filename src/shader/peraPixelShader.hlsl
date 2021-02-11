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
		return texNormal.Sample(smp, input.uv / 0.3);
	}
	else if (input.uv.x < 0.3 && input.uv.y < 0.6)
	{
		return texHighLum.Sample(smp, (input.uv - float2(0, 0.3)) / 0.3);
	}
	else if (input.uv.x < 0.3 && input.uv.y < 0.9)
	{
		return texShrinkHighLum.Sample(smp, (input.uv - float2(0, 0.6)) / 0.3);
	}

	
	float2 inputuv = input.uv -float2(0.1, 0);

	float w, h, miplevels;
	tex.GetDimensions(0, w, h, miplevels);
	float dx = 1.0 / w;
	float dy = 1.0 / h;
	
	float4 bloomAccum = float4(0, 0, 0, 0);
	float2 uvSize = float2(1, 0.5);
	float2 uvOfst = float2(0, 0);
	for (int i = 0; i < 3; ++i) {
		bloomAccum += Get5x5GaussianBlur(texShrinkHighLum, smp, inputuv * uvSize + uvOfst, dx, dy, float4(uvOfst, uvOfst + uvSize));
		uvOfst.y += uvSize.y;
		uvSize *= 0.5f;
	}
	

	return tex.Sample(smp, inputuv) + Get5x5GaussianBlur(texHighLum, smp, inputuv, dx, dy, float4(0, 0, 1, 1)) +saturate(bloomAccum);
	
	
	//あってんのか分からぬ
	/*
	//画面真ん中からの深度の差を測る
	float depthDiff = abs(depthTex.Sample(smp, float2(0.5,0.5)) - depthTex.Sample(smp, input.uv));
	depthDiff = pow(depthDiff,0.5f);
	uvSize = float2(1, 0.5);
	uvOfst = float2(0, 0);
	float t = depthDiff * 8;
	float no;
	t = modf(t, no);
	float4 retColor[2];
	retColor[0] = tex.Sample(smp, input.uv);//通常テクスチャ
	if (no == 0.0f) {
		retColor[1] = Get5x5GaussianBlur(texShrink, smp, input.uv * uvSize + uvOfst, dx, dy, float4(uvOfst, uvOfst + uvSize));
	}
	else {
		for (int i = 1; i <= 8; ++i) {
			if (i - no <= 0)continue;
			retColor[i - no] = Get5x5GaussianBlur(texShrink, smp, input.uv * uvSize + uvOfst, dx, dy, float4(uvOfst, uvOfst + uvSize));
			uvOfst.y += uvSize.y;
			uvSize *= 0.5f;
			if (i - no >= 1) {
				break;
			}
		}
	}
	return lerp(retColor[0],retColor[1],t);
	*/
	
}


BlurOutput BlurPS(Output input) 
{
	float w,h,miplevels;
	tex.GetDimensions(0, w, h, miplevels);
	float dx = 1.0 / w;
	float dy = 1.0 / h;
	BlurOutput ret;
	ret.highLum = Get5x5GaussianBlur(texHighLum, smp, input.uv, dx, dy, float4(0, 0, 1, 1));
	ret.col = Get5x5GaussianBlur(tex, smp, input.uv, dx, dy, float4(0, 0, 1, 1));// tex.Sample(smp, input.uv);

	return ret;
}