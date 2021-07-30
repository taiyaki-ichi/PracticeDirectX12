
RWTexture2D<float> destinationImage : register(u0);
Texture2D<float> depthImage : register(t0);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float2 pos = DTid.xy;

	if (destinationImage[pos] > -depthImage[pos])
		destinationImage[pos] = -depthImage[pos];
	else if (destinationImage[DTid.xy] < 0.f)
		destinationImage[DTid.xy] += 0.02f;

	if (destinationImage[DTid.xy] > 0.f)
		destinationImage[DTid.xy] = 0.f;
}