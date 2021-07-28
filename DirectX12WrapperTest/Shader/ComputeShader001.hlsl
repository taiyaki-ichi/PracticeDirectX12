Texture2D<float4> sourceImage : register(t0);
RWTexture2D<float4> destinationImage : register(u0);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	int k = 0;
	float3 pixcels[9];
	for (int y = -1; y <= 1; y++) {
		for (int x = -1; x <= 1; x++) {
			float2 index = DTid.xy;
			index += float2(x, y);
			pixcels[k] = sourceImage[index].xyz;
			k++;
		}
	}

	float3 sobelH = pixcels[0] * -1 + pixcels[2] * 1 + pixcels[3] * -2 + pixcels[5] * 2 + pixcels[6] * -1 + pixcels[8] * 1;
	float3 sobelV = pixcels[0] * -1 + pixcels[1] * -2 + pixcels[2] * -1 + pixcels[6] * 1 + pixcels[7] * 2 + pixcels[8] * 1;

	float4 color = float4(sqrt(sobelV * sobelV + sobelH * sobelH), 1);
	destinationImage[DTid.xy] = color;
}