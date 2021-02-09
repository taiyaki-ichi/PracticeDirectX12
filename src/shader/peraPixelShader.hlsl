#include"peraHeader.hlsli"

float4 main(Output input) : SV_TARGET
{
	if (input.uv.x < 0.4 && input.uv.y < 0.4)
	{
		return texNormal.Sample(smp, input.uv * 2.5);
	}

	return tex.Sample(smp,input.uv);

	//float4 col = tex.Sample(smp, input.uv);
	//float Y = dot(col.rgb, float3(0.299, 0.587, 0.114));
	//return float4(Y, Y, Y, 1);

/*
	float w, h, level;
	tex.GetDimensions(0, w, h, level);
	float dx = 1.0f / w;
	float dy = 1.0f / h;
	float4 ret = float4(0, 0, 0, 0);
	float4 col = tex.Sample(smp, input.uv);

	ret += tex.Sample(smp, input.uv + float2(0, -2 * dy)) * -1;//ã
	ret += tex.Sample(smp, input.uv + float2(-2 * dx, 0)) * -1;//¶
	ret += tex.Sample(smp, input.uv) * 4;//Ž©•ª
	ret += tex.Sample(smp, input.uv + float2(2 * dx, 0)) * -1;//‰E
	ret += tex.Sample(smp, input.uv + float2(0, 2 * dy)) * -1;//‰º
	//‚±‚±‚Å”½“]
	float Y = dot(ret.rgb, float3(0.299, 0.587, 0.114));
	Y = pow(1.0f - Y, 30.0f);
	Y = step(0.2, Y);
	return float4(Y, Y, Y, col.a);
	*/
}