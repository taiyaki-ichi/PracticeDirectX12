
RWTexture2D<float> heightMap : register(u0);
RWTexture2D<float> elapsedTimeMap : register(u1);
Texture2D<float> depthImage : register(t0);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float2 pos = DTid.xy;

	if (heightMap[pos] > -depthImage[pos]) {
		heightMap[pos] = -depthImage[pos];
		elapsedTimeMap[pos] = 100.f;
	}
	else if (heightMap[DTid.xy] < 0.f) {
		heightMap[DTid.xy] += 0.01f;
		elapsedTimeMap[pos] -= 0.5f;
	}

	if (heightMap[DTid.xy] > 0.f)
		heightMap[DTid.xy] = 0.f;

	//
	if (elapsedTimeMap[pos] < 0.f)
		elapsedTimeMap[pos] = 0.f;
}