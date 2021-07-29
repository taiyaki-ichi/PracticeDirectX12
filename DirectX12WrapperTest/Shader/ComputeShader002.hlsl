
cbuffer ComputeData : register(b0) {
	float2 ballPos;
	float radius;
};

RWTexture2D<float> destinationImage : register(u0);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float2 pos = DTid.xy;

	float l = length(pos - ballPos);
	if (l <= radius) {
		float rate = 1.f - l / radius;
		float y = sqrt(radius * radius - l * l);
		if (destinationImage[DTid.xy] < y)
			destinationImage[DTid.xy] = y;
	}
	else if (destinationImage[DTid.xy] > 0.f)
	{
		destinationImage[DTid.xy] -= 0.1f;
	}

	if (destinationImage[DTid.xy] < 0.f)
		destinationImage[DTid.xy] = 0.f;


}