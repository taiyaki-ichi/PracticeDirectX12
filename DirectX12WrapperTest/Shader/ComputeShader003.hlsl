
Texture2D<float> heightImage : register(t0);
RWTexture2D<float4> destinationImage : register(u0);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float2 id_x_1 = float2(DTid.x + 1, DTid.y);
	float2 id_x_2 = float2(DTid.x - 1, DTid.y);

	float3 x = float3(id_x_1.x, heightImage[id_x_1.xy], id_x_1.y) - float3(id_x_2.x, heightImage[id_x_2.xy], id_x_2.y);

	float2 id_y_1 = float2(DTid.x, DTid.y - 1);
	float2 id_y_2 = float2(DTid.x, DTid.y + 1);

	float3 y = float3(id_y_1.x, heightImage[id_y_1.xy], id_y_1.y) - float3(id_y_2.x, heightImage[id_y_2.xy], id_y_2.y);

	float3 n = normalize(cross(x, y));

	destinationImage[DTid.xy] = float4(n, 1.f);
}