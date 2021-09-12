

#define WIDTH_HEIGHT_EVEN 0
#define WIDTH_ODD_HEIGHT_EVEN 1
#define WIDTH_EVEN_HEIGHT_ODD 2
#define WIDTH_HEIGHT_ODD 3

RWTexture2D<float4> OutMip1 : register(u0);
RWTexture2D<float4> OutMip2 : register(u1);
RWTexture2D<float4> OutMip3 : register(u2);
RWTexture2D<float4> OutMip4 : register(u3);

Texture2D<float4> SrcMip : register(t0);
SamplerState LinearClampSampler :register(s0);

cbuffer CB0 : register(b0)
{
	uint SrcMipLevel;
	uint NumMipLevels;
	uint SrcDimension;
	bool IsSRGB;//
	float2 TexelSize;
}

struct ComputeShaderInput
{
	uint3 GroupID : SV_GroupID;
	uint3 GroupThreadID : SV_GroupThreadID;
	uint3 DispatchThreadID : SV_DispatchThreadID;
	uint GroupIndex : SV_GroupIndex;
};


groupshared float gs_R[64];
groupshared float gs_G[64];
groupshared float gs_B[64];
groupshared float gs_A[64];

void StoreColor(uint Index, float4 Color)
{
	gs_R[Index] = Color.r;
	gs_G[Index] = Color.g;
	gs_B[Index] = Color.b;
	gs_A[Index] = Color.a;
}

float4 LoadColor(uint Index)
{
	return float4(gs_R[Index], gs_G[Index], gs_B[Index], gs_A[Index]);
}


[numthreads(8,8,1)]
void main(ComputeShaderInput IN)
{
	float4 Src1 = (float4)0;

	switch (SrcDimension)
	{

	case WIDTH_HEIGHT_EVEN:
	{
		float2 UV = TexelSize * (IN.DispatchThreadID.xy + 0.5);
		Src1 = SrcMip.SampleLevel(LinearClampSampler, UV, SrcMipLevel);
	}
		break;
	

	case WIDTH_ODD_HEIGHT_EVEN:
	{
		float2 UV1 = TexelSize * (IN.DispatchThreadID.xy + float2(0.25, 0.5));
		float2 Off = TexelSize * float2(0.5, 0.0);
		Src1 = 0.5 * (SrcMip.SampleLevel(LinearClampSampler, UV1, SrcMipLevel) +
			SrcMip.SampleLevel(LinearClampSampler, UV1 + Off, SrcMipLevel));
	}
		break;
	

	case WIDTH_EVEN_HEIGHT_ODD:
	{
		float2 UV1 = TexelSize * (IN.DispatchThreadID.xy + float2(0.5, 0.25));
		float2 Off = TexelSize * float2(0.0, 0.5);
		Src1 = 0.5 * (SrcMip.SampleLevel(LinearClampSampler, UV1, SrcMipLevel) +
			SrcMip.SampleLevel(LinearClampSampler, UV1 + Off, SrcMipLevel));
	}
		break;

	case WIDTH_HEIGHT_ODD:
	{
		float2 UV1 = TexelSize * (IN.DispatchThreadID.xy + float2(0.25, 0.25));
		float2 Off = TexelSize * 0.5;

		Src1 = SrcMip.SampleLevel(LinearClampSampler, UV1, SrcMipLevel);
		Src1 += SrcMip.SampleLevel(LinearClampSampler, UV1 + float2(Off.x, 0.0), SrcMipLevel);
		Src1 += SrcMip.SampleLevel(LinearClampSampler, UV1 + float2(0.0, Off.y), SrcMipLevel);
		Src1 += SrcMip.SampleLevel(LinearClampSampler, UV1 + float2(Off.x, Off.y), SrcMipLevel);
		Src1 *= 0.25;
	}
	break;

	}

	//OutMip1[IN.DispatchThreadID.xy] = float4(1, 0, 0, 1);
	OutMip1[IN.DispatchThreadID.xy] = Src1;

	if (NumMipLevels == 1)
		return;

	StoreColor(IN.GroupIndex, Src1);

	//ëºÇÃÉOÉãÅ[ÉvÇë“Ç¬
	GroupMemoryBarrierWithGroupSync();

	if ((IN.GroupIndex & 0x9) == 0)
	{
		float4 Src2 = LoadColor(IN.GroupIndex + 0x01);
		float4 Src3 = LoadColor(IN.GroupIndex + 0x08);
		float4 Src4 = LoadColor(IN.GroupIndex + 0x09);
		Src1 = 0.25 * (Src1 + Src2 + Src3 + Src4);

		//OutMip2[IN.DispatchThreadID.xy / 2] = float4(0, 1, 0, 1);
		OutMip2[IN.DispatchThreadID.xy / 2] = Src1;
		StoreColor(IN.GroupIndex, Src1);
	}

	if (NumMipLevels == 2)
		return;

	//ë“Çø
	GroupMemoryBarrierWithGroupSync();

	if ((IN.GroupIndex & 0x1b) == 0)
	{
		float4 Src2 = LoadColor(IN.GroupIndex + 0x02);
		float4 Src3 = LoadColor(IN.GroupIndex + 0x10);
		float4 Src4 = LoadColor(IN.GroupIndex + 0x12);
		Src1 = 0.25 * (Src1 + Src2 + Src3 + Src4);

		//OutMip3[IN.DispatchThreadID.xy / 4] = float4(0, 0, 1, 1);
		OutMip3[IN.DispatchThreadID.xy / 4] = Src1;
		StoreColor(IN.GroupIndex, Src1);
	}

	
	if (NumMipLevels == 3)
		return;

	GroupMemoryBarrierWithGroupSync();

	if (IN.GroupIndex == 0)
	{
		float4 Src2 = LoadColor(IN.GroupIndex + 0x04);
		float4 Src3 = LoadColor(IN.GroupIndex + 0x20);
		float4 Src4 = LoadColor(IN.GroupIndex + 0x24);
		Src1 = 0.25 * (Src1 + Src2 + Src3 + Src4);

		//OutMip4[IN.DispatchThreadID.xy / 8] = float4(1, 1, 0, 1);
		OutMip4[IN.DispatchThreadID.xy / 8] = Src1;
	}

}
