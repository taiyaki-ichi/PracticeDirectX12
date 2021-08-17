
float2 main(float4 pos : SV_POSITION) : SV_TARGET
{
	float z = pos.z / pos.w;
	return float2(z, z * z);
}