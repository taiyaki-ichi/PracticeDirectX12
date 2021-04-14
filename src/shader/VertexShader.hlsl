#include"BasicType.hlsli"


BasicType main(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD, uint boneTypeFlag : BONE_TYPE_FLAG, min16uint4 boneno : BONENO, min16uint4 weight : WEIGHT, uint instNo : SV_InstanceId)
{

	BasicType output;//ピクセルシェーダへ渡す値

	if (boneTypeFlag == 0) {
		pos = mul(bones[boneno[0]], pos);
	}
	else if (boneTypeFlag == 1) {
		matrix bm = bones[boneno[0]] * weight[0] + bones[boneno[1]] * (1 - weight[0]);
		pos = mul(bm, pos);
	}
	else if (boneTypeFlag == 2) {
		float sumWeight = 0;
		for (int i = 0; i < 4; i++)
			sumWeight += weight[i];
		matrix bm =
			bones[boneno[0]] * weight[0] / sumWeight +
			bones[boneno[1]] * weight[1] / sumWeight +
			bones[boneno[2]] * weight[2] / sumWeight +
			bones[boneno[3]] * weight[3] / sumWeight;
		pos = mul(bm, pos);
	}

	pos = mul(world, pos);

	if (instNo == 1)
	{
		//ワールド座標をかけてから
		pos = mul(shadow, pos);
	}

	output.svpos = mul(mul(proj, view), pos);//シェーダでは列優先なので注意
	output.pos = pos;
	normal.w = 0;//ここ重要(平行移動成分を無効にする)
	output.normal = mul(world, normal);//法線にもワールド変換を行う
	output.vnormal = mul(view, output.normal);
	output.uv = uv;
	output.ray = normalize(pos.xyz - mul(view, eye));//視線ベクトル

	output.instNo = instNo;
	
	output.tpos = mul(lightCamera, output.pos);

	return output;
}


float4 shadowVS(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD) : SV_POSITION
{
	return mul(lightCamera,mul(world,pos));
}