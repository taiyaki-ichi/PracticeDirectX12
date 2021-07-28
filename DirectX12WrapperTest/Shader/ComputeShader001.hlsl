Texture2D<float4> sourceImage : register(t0);
RWTexture2D<float4> destinationImage : register(u0);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	if (sourceImage[DTid.xy].w > 0.0)
	{
		float3x3 toSepia = float3x3(
			0.393, 0.349, 0.272,
			0.769, 0.686, 0.535,
			0.189, 0.168, 0.131
			);
		float3 color = mul(sourceImage[DTid.xy].xyz, toSepia);
		destinationImage[DTid.xy] = float4(color, 1);
		//destinationImage[DTid.xy] = float4(1, 0, 0, 1);
	}
}