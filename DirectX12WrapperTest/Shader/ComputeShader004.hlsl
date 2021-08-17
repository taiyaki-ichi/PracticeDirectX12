
Texture2D<float2> srcShadowMap : register(t0);
RWTexture2D<float2> dstShadowMap : register(u0);

struct ShadowMapData
{
	float width;
	float height;
};

ConstantBuffer<ShadowMapData> shadowMapData : register(b0);

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
	const float dx = 1.0 / shadowMapData.width;
	const float dy = 1.0 / shadowMapData.height;

	float l1 = -dx, l2 = -2 * dx;
	float r1 = dx, r2 = 2 * dx;
	float u1 = -dy, u2 = -2 * dy;
	float d1 = dy, d2 = 2 * dy;

	dstShadowMap[id.xy] = (
		srcShadowMap[id.xy + float2(l2, u2)]
		+ srcShadowMap[id.xy + float2(l1, u2)] * 4.f
		+ srcShadowMap[id.xy + float2(0, u2)] * 6.f
		+ srcShadowMap[id.xy + float2(r1, u2)] * 4.f
		+ srcShadowMap[id.xy + float2(r2, u2)]

		+ srcShadowMap[id.xy + float2(l2, u1)] * 4.f
		+ srcShadowMap[id.xy + float2(l1, u1)] * 16.f
		+ srcShadowMap[id.xy + float2(0, u1)] * 24.f
		+ srcShadowMap[id.xy + float2(r1, u1)] * 16.f
		+ srcShadowMap[id.xy + float2(r2, u1)] * 4.f

		+ srcShadowMap[id.xy + float2(l2, 0)] * 6.f
		+ srcShadowMap[id.xy + float2(l1, 0)] * 24.f
		+ srcShadowMap[id.xy] * 36.f
		+ srcShadowMap[id.xy + float2(r1, 0)] * 24.f
		+ srcShadowMap[id.xy + float2(r2, 0)] * 6.f

		+ srcShadowMap[id.xy + float2(l2, d1)] * 4.f
		+ srcShadowMap[id.xy + float2(l1, d1)] * 16.f
		+ srcShadowMap[id.xy + float2(0, d1)] * 24.f
		+ srcShadowMap[id.xy + float2(r1, d1)] * 16.f
		+ srcShadowMap[id.xy + float2(r2, d1)] * 4.f

		+ srcShadowMap[id.xy + float2(l2, d2)]
		+ srcShadowMap[id.xy + float2(l1, d2)] * 4.f
		+ srcShadowMap[id.xy + float2(0, d2)] * 6.f
		+ srcShadowMap[id.xy + float2(r1, d2)] * 4.f
		+ srcShadowMap[id.xy + float2(r2, d2)]

		) / 256.f;
		
}